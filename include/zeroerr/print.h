#pragma once

#include "zeroerr/config.h"

#include <ostream>
#include <string>
#include <type_traits>
#include <memory>
#include <complex>

#ifdef __GNUG__
#include <cxxabi.h>
#endif

namespace zeroerr
{


namespace detail {

// C++11 void_t
template<typename... Ts> using void_t = void;


// Some utility structs to check template specialization
template<typename Test, template<typename...> class Ref>
struct is_specialization : std::false_type {};

template<template<typename...> class Ref, typename... Args>
struct is_specialization<Ref<Args...>, Ref> : std::true_type {};


// Check if a type is stream writable, i.e., std::cout << foo;
// Usage: is_streamable<std::ostream, int>::value
template<typename S, typename T, typename = void>
struct is_streamable: std::false_type {};

template<typename S, typename T>
struct is_streamable<S, T,
           void_t<decltype( std::declval<S&>() << std::declval<T>() )> >
  : std::true_type {};


// Check if a type is a container type
// Usage: is_container<std::vector<int>>::value
template<typename T, typename = void>
struct is_container : std::false_type{};

template<typename T>
struct is_container<T,
    void_t<
        decltype(std::declval<T>().begin()), 
        decltype(std::declval<T>().end()),
        typename T::value_type> > 
    : std::true_type {};

// Check if a type is a string type
template< class T >
struct is_string : std::integral_constant<bool,
                     std::is_same<T, std::string>::value 
                   ||std::is_same<T, const char*>::value
#if ZEROERR_CXX_STANDARD >= 17
                   ||std::is_same<T, std::string_view>::value
#endif
                  > {};


// Check if a type is a complex type
template< class T> struct is_complex : std::false_type {};
template< class T> struct is_complex<const T> : is_complex<T> {};
template< class T> struct is_complex<volatile T> : is_complex<T> {};
template< class T> struct is_complex<volatile const T> : is_complex<T> {};
template< class T> struct is_complex< std::complex<T> > : std::true_type {};

// Check if a type can use arr[0] like an array
template<typename T, typename = void>
struct is_array: std::false_type {};

template<typename T>
struct is_array<T, void_t<decltype(std::declval<T>()[0])> > : std::true_type {};


// Check if a type has the element type as std::pair
template<typename T, typename = void>
struct ele_type_is_pair : std::false_type {};

template<typename T>
struct ele_type_is_pair<T, 
    void_t<
        typename T::value_type,
        decltype(std::declval<typename T::value_type>().first),
        decltype(std::declval<typename T::value_type>().second)> >
    : std::true_type {};



template <unsigned N> struct rank : rank<N - 1> {};
template <>           struct rank<0> {};


template<std::size_t...> struct seq{};

template<std::size_t N, std::size_t... Is>
struct gen_seq : gen_seq<N-1, N-1, Is...>{};

template<std::size_t... Is>
struct gen_seq<0, Is...> : seq<Is...>{};



} // namespace detail



struct Printer {
    static const unsigned max_rank = 3;

    template<typename T, typename... V>
    void operator()(T value, V... others) {
        print(std::forward<T>(value), 0, " ", detail::rank<max_rank>{});
        (*this)(std::forward<V>(others)...);    
    }

    template<typename T>
    void operator()(T value) {
        print(std::forward<T>(value), 0, " ", detail::rank<max_rank>{});
        os << std::endl;
    }
    
    template<typename T> void
    operator()(std::initializer_list<T> value) {
        print(value, 0, " ", detail::rank<max_rank>{});
        os << std::endl;
    }

    Printer(std::ostream& os);

    bool isColorful = true;  // colorful output
    bool isCompact  = false; // compact mode
    bool isQuoted   = true;  // string is quoted
    int  indent     = 2;
protected:
    std::ostream& os;


#include "zeroerr/_push"

    ENABLE_IF(IS_INT || IS_FLOAT)
    print(T value, unsigned level, const char* lb, detail::rank<0> r) {
        os << tab(level) << value << lb;
    } 

    ENABLE_IF(IS_POINTER)
    print(T value, unsigned level, const char* lb, detail::rank<0> r) {
        if (value == nullptr) 
            os << tab(level) << "nullptr" << lb;
        else 
            os << tab(level) << "<" << type(value) << " at " << value << ">" << lb;
    }


    ENABLE_IF(IS_CLASS)
    print(T value, unsigned level, const char* lb, detail::rank<0> r) {
        os << tab(level) << "Object " << type(value) << lb;
    }


    ENABLE_IF(IS_AUTOPTR)
    print(T value, unsigned level, const char* lb, detail::rank<2> r) {
        if (value.get() == nullptr) 
            os << tab(level) << "nullptr" << lb;
        else 
            os << tab(level) << "<" << type(value) << " at " << value.get() << ">" << lb;
    }

    ENABLE_IF(IS_CONTAINER)
    print(const T& value, unsigned level, const char* lb, detail::rank<1> r) {
        os << tab(level) << "{" << (isCompact? "" : "\n");
        bool last = false;
        for (auto iter = value.begin(); iter != value.end(); ++iter) {
            if (std::next(iter) == value.end()) last = true;
            print(*iter, level+1, isCompact? (last?"": ", "): "\n", detail::rank<max_rank>{});
        }
        os << tab(level) << "}" << lb;
    }

    ENABLE_IF(IS_CHAR || IS_WCHAR)
    print(T value, unsigned level, const char* lb, detail::rank<1> r) {
        os << tab(level) << '\'' << value << '\'' << lb;
    }

    ENABLE_IF(IS_CLASS && IS_STREAMABLE)
    print(T value, unsigned level, const char* lb, detail::rank<1> r) {
        os << tab(level) << value << lb;
    }

    ENABLE_IF(IS_STRING)
    print(T value, unsigned level, const char* lb, detail::rank<3> r) {
        os << tab(level) << quote() << value << quote() << lb;
    }

    ENABLE_IF(IS_CONTAINER && IS_ARRAY)
    print(const T& value, unsigned level, const char* lb, detail::rank<2> r) {
        os << tab(level) << "[";
        bool last = false;
        for (auto iter = value.begin(); iter != value.end(); ++iter) {
            if (std::next(iter) == value.end()) last = true;
            print(*iter, 0, last? "": ", ", detail::rank<max_rank>{});
        }
        os << tab(level) << "]" << lb;
    }

    ENABLE_IF(IS_CONTAINER && IS_MAP)
    print(const T& value, unsigned level, const char* lb, detail::rank<3> r) {
        os << tab(level) << "{" << (isCompact? "" : "\n");
        bool last = false;
        for (auto iter = value.begin(); iter != value.end(); ++iter) {
            if (std::next(iter) == value.end()) last = true;
            print(iter->first, level+1, " : ", detail::rank<max_rank>{});
            print(iter->second, level+1, isCompact? (last?"": ", "): "\n", detail::rank<max_rank>{});
        }
        os << tab(level) << "}" << lb;
    }

    ENABLE_IF(IS_COMPLEX)
    print(T value, unsigned level, const char* lb, detail::rank<2> r) {
        os << tab(level) << "(" << value.real() << "+" << value.imag() << "i)" << lb;
    }

    ENABLE_IF(IS_BOOL)
    print(T value, unsigned level, const char* lb, detail::rank<2> r) {
        os << tab(level) << (value ? "true" : "false") << lb;
    }


    template<class TupType, size_t... I>
    void print_tuple(const TupType& _tup, unsigned level, const char* lb, detail::seq<I...>)
    {
        int a[] = {(os << (I == 0? "" : ", ") << std::get<I>(_tup), 0)...};
    }

    template<class... Args> void
    print(const std::tuple<Args...>& value, unsigned level, const char* lb, detail::rank<2> r) {
        os << tab(level) << "(";
        print_tuple(value, level, isCompact? " ": "\n", detail::gen_seq<sizeof...(Args)>{});
        os << ")" << lb;
    }




#include "zeroerr/_pop"

    std::string tab(unsigned level) { return std::string(level*indent, ' '); }
    const char* quote() { return isQuoted? "\"" : ""; }

    std::string demangle(const char* name) {
#ifdef __GNUG__
      int status = -4;
      std::unique_ptr<char, void(*)(void*)> res {
        abi::__cxa_demangle(name, NULL, NULL, &status),
        std::free
      };
      return (status==0) ? res.get() : name;
#else
      return name;
#endif
    }

    template <class T>
    std::string type(const T& t) {
      return demangle(typeid(t).name());
    }


    
};

extern Printer& getStdoutPrinter();
extern Printer& getStderrPrinter();



} // namespace zeroerr
