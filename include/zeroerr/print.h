#pragma once
#include "zeroerr/internal/config.h"

#include "zeroerr/color.h"

#include <ostream>
#include <sstream>
#include <string>
#include <tuple>  // this should be removed
#include <type_traits>
#ifdef __GNUG__
#include <cxxabi.h>
#endif

#if defined(ZEROERR_ENABLE_PFR) && (ZEROERR_CXX_STANDARD >= 14)
#include "pfr.hpp"
#endif

#if defined(ZEROERR_ENABLE_MAGIC_ENUM) && (ZEROERR_CXX_STANDARD >= 17)
#include "magic_enum.hpp"
#endif

// those predefines can help to avoid include too many headers
namespace std {
template <typename T>
class complex;

template <class T, class Deleter>
class unique_ptr;

template <class T>
class shared_ptr;

template <class T>
class weak_ptr;

}  // namespace std

namespace zeroerr {

#pragma region type traits

struct Printer;

template <unsigned N>
struct rank : rank<N - 1> {};
template <>
struct rank<0> {};
constexpr unsigned max_rank = 5;


template <typename T>
void PrinterExt(Printer&, T, unsigned, const char*, rank<0>);


namespace detail {

// C++11 void_t
template <typename... Ts>
using void_t = void;


// Some utility structs to check template specialization
template <typename Test, template <typename...> class Ref>
struct is_specialization : std::false_type {};

template <template <typename...> class Ref, typename... Args>
struct is_specialization<Ref<Args...>, Ref> : std::true_type {};


// Check if a type is stream writable, i.e., std::cout << foo;
// Usage: is_streamable<std::ostream, int>::value
template <typename S, typename T, typename = void>
struct is_streamable : std::false_type {};

template <typename S, typename T>
struct is_streamable<S, T, void_t<decltype(std::declval<S&>() << std::declval<T>())>>
    : std::true_type {};


// Check if a type is a container type
// Usage: is_container<std::vector<int>>::value
template <typename T, typename = void>
struct is_container : std::false_type {};

template <typename T>
struct is_container<T,
                    void_t<decltype(std::declval<T>().begin()), decltype(std::declval<T>().end())>>
    : std::true_type {};

#if ZEROERR_CXX_STANDARD >= 17
#define ZEROERR_STRING_VIEW std::is_same<T, std::string_view>::value
#else
#define ZEROERR_STRING_VIEW 0
#endif

// Check if a type is a string type
template <class T>
struct is_string
    : std::integral_constant<bool, std::is_same<T, std::string>::value ||
                                       std::is_same<T, const char*>::value || ZEROERR_STRING_VIEW> {
};


// Check if a type can use arr[0] like an array
template <typename T, typename = void>
struct is_array : std::false_type {};

template <typename T>
struct is_array<T, void_t<decltype(std::declval<T>()[0])>> : std::true_type {};


// Check if a type has the element type as std::pair
template <typename T, typename = void>
struct ele_type_is_pair : std::false_type {};

template <typename T>
struct ele_type_is_pair<
    T, void_t<typename T::value_type, decltype(std::declval<typename T::value_type>().first),
              decltype(std::declval<typename T::value_type>().second)>> : std::true_type {};

template <typename T, typename = void>
struct has_extension : std::false_type {};

template <typename T>
struct has_extension<
    T, void_t<decltype(zeroerr::PrinterExt(std::declval<zeroerr::Printer&>(), std::declval<T&>(), 0,
                                           nullptr, zeroerr::rank<zeroerr::max_rank>()))>>
    : std::true_type {};


// Generate sequence of integers from 0 to N-1
// Usage: detail::gen_seq<N>  then use <size_t... I> to match it
template <std::size_t...>
struct seq {};

template <std::size_t N, std::size_t... Is>
struct gen_seq : gen_seq<N - 1, N - 1, Is...> {};

template <std::size_t... Is>
struct gen_seq<0, Is...> : seq<Is...> {};


#define ZEROERR_ENABLE_IF(x) \
    template <typename T>    \
    typename std::enable_if<x, void>::type
#define ZEROERR_IS_INT        std::is_integral<T>::value
#define ZEROERR_IS_FLOAT      std::is_floating_point<T>::value
#define ZEROERR_IS_CONTAINER  detail::is_container<T>::value
#define ZEROERR_IS_STRING     detail::is_string<T>::value
#define ZEROERR_IS_POINTER    std::is_pointer<T>::value
#define ZEROERR_IS_CHAR       std::is_same<T, char>::value
#define ZEROERR_IS_WCHAR      std::is_same<T, wchar_t>::value
#define ZEROERR_IS_CLASS      std::is_class<T>::value
#define ZEROERR_IS_STREAMABLE detail::is_streamable<std::ostream, T>::value
#define ZEROERR_IS_ARRAY      detail::is_array<T>::value
#define ZEROERR_IS_COMPLEX    detail::is_specialization<T, std::complex>::value
#define ZEROERR_IS_BOOL       std::is_same<T, bool>::value
#define ZEROERR_IS_AUTOPTR                                   \
    (detail::is_specialization<T, std::unique_ptr>::value || \
     detail::is_specialization<T, std::shared_ptr>::value || \
     detail::is_specialization<T, std::weak_ptr>::value)
#define ZEROERR_IS_MAP detail::ele_type_is_pair<T>::value
#define ZEROERR_IS_POD std::is_standard_layout<T>::value
#define ZEROERR_IS_EXT detail::has_extension<T>::value

}  // namespace detail
#pragma endregion


/**
 * @brief A functor class Printer for printing a value of any type.
 *
 * This class can print values with all basic types, pointers, STL containers, tuple, optional, and
 * variant values. Any class that is streamable can be printed. POD structs can be supported using
 * third-party library Boost.PFR and enum can be supported using magic_enum.
 */
struct Printer {
    template <typename T, typename... V>
    Printer& operator()(T value, V... others) {
        PrinterExt(*this, std::forward<T>(value), 0, " ", rank<max_rank>{});
        (*this)(std::forward<V>(others)...);
        return *this;
    }

    template <typename T>
    Printer& operator()(T value) {
        PrinterExt(*this, std::forward<T>(value), 0, "", rank<max_rank>{});
        os << line_break;
        os.flush();
        return *this;
    }

    template <typename T>
    Printer& operator()(std::initializer_list<T> value) {
        PrinterExt(*this, value, 0, " ", rank<max_rank>{});
        os << line_break;
        os.flush();
        return *this;
    }

    Printer(std::ostream& os) : os(os) {}
    Printer() : os(*new std::stringstream()) { use_stringstream = true; }
    ~Printer() {
        if (use_stringstream) delete &os;
    }

    bool          isColorful = true;   // colorful output
    bool          isCompact  = false;  // compact mode
    bool          isQuoted   = true;   // string is quoted
    int           indent     = 2;
    const char*   line_break = "\n";
    std::ostream& os;
    bool          use_stringstream = false;

    template <class T>
    static std::string type(const T& t) {
        return demangle(typeid(t).name());
    }


    ZEROERR_ENABLE_IF(ZEROERR_IS_INT || ZEROERR_IS_FLOAT)
    print(T value, unsigned level, const char* lb, rank<0>) { os << tab(level) << value << lb; }

    ZEROERR_ENABLE_IF(ZEROERR_IS_POINTER)
    print(T value, unsigned level, const char* lb, rank<0>) {
        if (value == nullptr)
            os << tab(level) << "nullptr" << lb;
        else
            os << tab(level) << "<" << type(value) << " at " << value << ">" << lb;
    }


    ZEROERR_ENABLE_IF(ZEROERR_IS_CLASS)
    print(T value, unsigned level, const char* lb, rank<0>) {
        os << tab(level) << "Object " << type(value) << lb;
    }


    ZEROERR_ENABLE_IF(ZEROERR_IS_CHAR || ZEROERR_IS_WCHAR)
    print(T value, unsigned level, const char* lb, rank<1>) {
        os << tab(level) << '\'' << value << '\'' << lb;
    }

#if defined(ZEROERR_ENABLE_PFR) && (ZEROERR_CXX_STANDARD >= 14)
    template <class StructType, size_t... I>
    void print_struct(const StructType& s, unsigned level, const char* lb, detail::seq<I...>) {
        int a[] = {(os << (I == 0 ? "" : ", ") << pfr::get<I>(s), 0)...};
    }

    ZEROERR_ENABLE_IF(ZEROERR_IS_CLASS&& ZEROERR_IS_POD)
    print(const T& value, unsigned level, const char* lb, rank<1>) {
        os << tab(level) << "{";
        print_struct(value, level, isCompact ? " " : line_break,
                     detail::gen_seq<pfr::tuple_size<T>::value>{});
        os << tab(level) << "}" << lb;
    }
#endif

    ZEROERR_ENABLE_IF(ZEROERR_IS_BOOL)
    print(T value, unsigned level, const char* lb, rank<2>) {
        os << tab(level) << (value ? "true" : "false") << lb;
    }

    ZEROERR_ENABLE_IF(ZEROERR_IS_CLASS&& ZEROERR_IS_STREAMABLE)
    print(T value, unsigned level, const char* lb, rank<2>) { os << tab(level) << value << lb; }


    ZEROERR_ENABLE_IF(ZEROERR_IS_CONTAINER)
    print(const T& value, unsigned level, const char* lb, rank<2>) {
        os << tab(level) << "{" << (isCompact ? "" : line_break);
        bool last = false;
        for (auto iter = value.begin(); iter != value.end(); ++iter) {
            if (std::next(iter) == value.end()) last = true;
            print(*iter, level + 1, isCompact ? (last ? "" : ", ") : line_break, rank<max_rank>{});
        }
        os << tab(level) << "}" << lb;
    }

    ZEROERR_ENABLE_IF(ZEROERR_IS_CONTAINER&& ZEROERR_IS_ARRAY)
    print(const T& value, unsigned level, const char* lb, rank<3>) {
        os << tab(level) << "[";
        bool last = false;
        for (auto iter = value.begin(); iter != value.end(); ++iter) {
            if (std::next(iter) == value.end()) last = true;
            print(*iter, 0, last ? "" : ", ", rank<max_rank>{});
        }
        os << tab(level) << "]" << lb;
    }


    ZEROERR_ENABLE_IF(ZEROERR_IS_AUTOPTR)
    print(T value, unsigned level, const char* lb, rank<3> r) {
        if (value.get() == nullptr)
            os << tab(level) << "nullptr" << lb;
        else
            os << tab(level) << "<" << type(value) << " at " << value.get() << ">" << lb;
    }

    ZEROERR_ENABLE_IF(ZEROERR_IS_CONTAINER&& ZEROERR_IS_MAP)
    print(const T& value, unsigned level, const char* lb, rank<4>) {
        os << tab(level) << "{" << (isCompact ? "" : line_break);
        bool last = false;
        for (auto iter = value.begin(); iter != value.end(); ++iter) {
            if (std::next(iter) == value.end()) last = true;
            print(iter->first, level + 1, " : ", rank<max_rank>{});
            print(iter->second, level + 1, isCompact ? (last ? "" : ", ") : line_break,
                  rank<max_rank>{});
        }
        os << tab(level) << "}" << lb;
    }

    ZEROERR_ENABLE_IF(ZEROERR_IS_COMPLEX)
    print(T value, unsigned level, const char* lb, rank<4>) {
        os << tab(level) << "(" << value.real() << "+" << value.imag() << "i)" << lb;
    }

    ZEROERR_ENABLE_IF(ZEROERR_IS_STRING)
    print(T value, unsigned level, const char* lb, rank<4>) {
        os << tab(level) << quote() << value << quote() << lb;
    }

    template <class TupType, size_t... I>
    inline void print_tuple(const TupType& _tup, unsigned level, const char* lb,
                            detail::seq<I...>) {
        int a[] = {(os << (I == 0 ? "" : ", ") << std::get<I>(_tup), 0)...};
    }

    template <class... Args>
    void print(const std::tuple<Args...>& value, unsigned level, const char* lb, rank<3> r) {
        os << tab(level) << "(";
        print_tuple(value, level, isCompact ? " " : line_break, detail::gen_seq<sizeof...(Args)>{});
        os << ")" << lb;
    }

    std::string tab(unsigned level) { return std::string((isCompact ? 0 : level * indent), ' '); }
    const char* quote() { return isQuoted ? "\"" : ""; }

    static std::string demangle(const char* name) {
#ifdef __GNUG__
        int         status = -4;
        char*       res    = abi::__cxa_demangle(name, NULL, NULL, &status);
        std::string ret    = (status == 0) ? res : name;
        std::free(res);
        return ret;
#else
        return name;
#endif
    }

    std::string str() const { return static_cast<std::stringstream&>(os).str(); }

    friend std::ostream& operator<<(std::ostream& os, const Printer& P) {
        if (P.use_stringstream) os << P.str();
        return os;
    }
};

/**
 * @brief PrinterExt is an extension of Printer that allows user to write custom rules for printing.
 * User can use SFINAE to extend PrinterExt, e.g.:
 * template <typename T>
 * typename std::enable_if<std::is_base_of<llvm::Function, T>::value, void>::type
 * PrinterExt(Printer& P, T* s, unsigned level, const char* lb, rank<3>);
 *
 * @tparam T the type of the object to be printed.
 * @param P Printer class
 * @param v the object to be printed.
 * @param level the indentation level.
 * @param lb  the line break.
 * @param r  the rank of the rule. 0 is lowest priority.
 */
template <class T>
void PrinterExt(Printer& P, T v, unsigned level, const char* lb, rank<0> r) {
    P.print(std::forward<T>(v), level, lb, rank<max_rank>{});
}

extern Printer& getStdoutPrinter();
extern Printer& getStderrPrinter();


}  // namespace zeroerr
