#pragma once
#include "zeroerr/internal/config.h"

#include "zeroerr/color.h"
#include "zeroerr/internal/typetraits.h"


#ifdef __GNUG__
#include <cxxabi.h>
#endif

#if defined(ZEROERR_ENABLE_PFR) && (ZEROERR_CXX_STANDARD >= 14)
#include "pfr.hpp"
#endif

#if defined(ZEROERR_ENABLE_MAGIC_ENUM) && (ZEROERR_CXX_STANDARD >= 17)
#include "magic_enum.hpp"
#endif

#if defined(ZEROERR_ENABLE_DSVIZ)
#include "dsviz.h"
#endif

ZEROERR_SUPPRESS_COMMON_WARNINGS_PUSH


namespace zeroerr {


constexpr unsigned max_rank = 5;


struct Printer;
template <typename T>
void PrinterExt(Printer&, T, unsigned, const char*, rank<0>);

namespace detail {

/**
 * @brief has_extension is a type trait to check if user defined PrinterExt for a type
 */
template <typename T, typename = void>
struct has_extension : std::false_type {};

template <typename T>
using has_printer_ext =
    void_t<decltype(zeroerr::PrinterExt(std::declval<zeroerr::Printer&>(), std::declval<T&>(), 0,
                                        nullptr, zeroerr::rank<zeroerr::max_rank>()))>;

template <typename T>
struct has_extension<T, has_printer_ext<T>> : std::true_type {};

}  // namespace detail


/**
 * @brief A functor class Printer for printing a value of any type.
 *
 * This class can print values with all basic types, pointers, STL containers, tuple, optional, and
 * variant values. Any class that is streamable can be printed. POD structs can be supported using
 * third-party library Boost.PFR and enum can be supported using magic_enum.
 */
struct Printer {
    template <typename... Args>
    Printer& operator()(Args&&... args) {
        check_stream();
        call(std::forward<Args>(args)...);
        return *this;
    }
    template <typename T>
    Printer& operator()(std::initializer_list<T>&& value) {
        check_stream();
        call(std::forward<decltype(value)>(value));
        return *this;
    }

    void check_stream() {
        if (use_stringstream && clear_stream_before_printing) {
            auto& ss = static_cast<std::stringstream&>(os);
            ss.str(std::string());
            ss.clear();
        }
    }

    template <typename T, typename... V>
    void call(T value, V... others) {
        PrinterExt(*this, std::forward<T>(value), 0, " ", rank<max_rank>{});
        call(std::forward<V>(others)...);
    }

    template <typename T>
    void call(T value) {
        PrinterExt(*this, std::forward<T>(value), 0, "", rank<max_rank>{});
        os << line_break;
        os.flush();
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
    bool          use_stringstream             = false;
    bool          clear_stream_before_printing = true;

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
        os << tab(level) << type(value) << lb;
    }


    ZEROERR_ENABLE_IF(ZEROERR_IS_CHAR || ZEROERR_IS_WCHAR)
    print(T value, unsigned level, const char* lb, rank<1>) {
        os << tab(level) << '\'' << value << '\'' << lb;
    }

#if defined(ZEROERR_ENABLE_PFR) && (ZEROERR_CXX_STANDARD >= 14)
    template <class StructType, unsigned... I>
    void print_struct(const StructType& s, unsigned, const char*, detail::seq<I...>) {
        int _[] = {(os << (I == 0 ? "" : ", ") << pfr::get<I>(s), 0)...};
        (void)_;
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
    print(T value, unsigned level, const char* lb, rank<3>) {
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

    template <class TupType>
    inline void print_tuple(const TupType&, unsigned, const char*, detail::seq<>) {}

    template <class TupType, unsigned... I>
    inline void print_tuple(const TupType& _tup, unsigned level, const char*, detail::seq<I...>) {
        int _[] = {(os << (I == 0 ? "" : ", "),
                    print(std::get<I>(_tup), level + 1, "", rank<max_rank>{}), 0)...};
        (void)_;
    }

    template <class... Args>
    void print(const std::tuple<Args...>& value, unsigned level, const char* lb, rank<3>) {
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

    std::string str() const {
        if (use_stringstream == false)
            throw std::runtime_error("Printer is not using stringstream");
        return static_cast<std::stringstream&>(os).str();
    }
    operator std::string() const { return str(); }

    friend std::ostream& operator<<(std::ostream& os, const Printer& P) {
        if (P.use_stringstream) os << P.str();
        return os;
    }
};

/**
 * @brief PrinterExt is an extension of Printer that allows user to write custom rules for printing.
 * @details
 * User can use SFINAE to extend PrinterExt, e.g.:
 * ```
 * template<typename T>
 * typename std::enable_if<std::is_base_of<llvm::Function, T>::value, void>::type
 * PrinterExt(Printer& P, T* s, unsigned level, const char* lb, rank<3>);
 * ```
 * @tparam T the type of the object to be printed.
 * @param P Printer class
 * @param v the object to be printed.
 * @param level the indentation level.
 * @param lb  the line break.
 * @param r  the rank of the rule. 0 is lowest priority.
 */
template <class T>
void PrinterExt(Printer& P, T v, unsigned level, const char* lb, rank<0>) {
    P.print(std::forward<T>(v), level, lb, rank<max_rank>{});
}

extern Printer& getStdoutPrinter();
extern Printer& getStderrPrinter();


}  // namespace zeroerr

ZEROERR_SUPPRESS_COMMON_WARNINGS_POP
