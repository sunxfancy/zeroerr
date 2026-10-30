#pragma once

// libc++ offers compatibility-only transitive includes through C++20.
// Suppress those while loading our explicit dependencies, then restore the
// caller's setting. This does not change standard-library types or ABI.
#pragma push_macro("_LIBCPP_REMOVE_TRANSITIVE_INCLUDES")
#ifndef _LIBCPP_REMOVE_TRANSITIVE_INCLUDES
#define _LIBCPP_REMOVE_TRANSITIVE_INCLUDES
#endif

#include "zeroerr/internal/config.h"

#include "zeroerr/color.h"
#include "zeroerr/internal/text.h"
#include "zeroerr/buffer.h"
#include "zeroerr/internal/typetraits.h"
#include <functional>

#include <memory>
#include <initializer_list>
#include <typeinfo>

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
void PrinterExt(Printer&, const T&, unsigned, const char*, rank<0>);

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

#if defined(ZEROERR_ENABLE_PFR) && ZEROERR_CXX_STANDARD >= 14 && !PFR_USE_CPP17 && !PFR_USE_LOOPHOLE
// Check the bundled classic backend's flat-field constraint before instantiating get().
// The primary template prevents probing strings and other non-trivial classes.
template <class T, bool = std::is_class<T>::value && std::is_trivial<T>::value &&
                          std::is_standard_layout<T>::value>
struct is_classic_pfr_printable : std::false_type {};

template <class T>
struct is_classic_pfr_printable<T, true>
    : std::integral_constant<bool, pfr::detail::is_flat_refelectable<T>(
          pfr::detail::make_index_sequence<pfr::detail::fields_count<T>()>{})> {};
#endif

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
        if (use_stringstream && clear_stream_before_printing) os.clear();
    }

    template <typename T, typename... V>
    void call(T&& value, V&&... others) {
        PrinterExt(*this, std::forward<T>(value), 0, " ", rank<max_rank>{});
        call(std::forward<V>(others)...);
    }

    template <typename T>
    void call(T&& value) {
        PrinterExt(*this, std::forward<T>(value), 0, "", rank<max_rank>{});
        os << line_break;
        os.flush();
    }

private:
    StringBuffer storage_;
public:
    Printer() : os(storage_) { use_stringstream = true; }
    template <class Stream>
    explicit Printer(Stream& stream) : os(storage_) { storage_.attach(stream); }
    Printer(const Printer& other) : storage_(other.storage_), isColorful(other.isColorful),
        isCompact(other.isCompact), isQuoted(other.isQuoted), indent(other.indent),
        line_break(other.line_break), os(storage_), use_stringstream(other.use_stringstream),
        clear_stream_before_printing(other.clear_stream_before_printing) {}

    bool          isColorful = true;   // colorful output
    bool          isCompact  = false;  // compact mode
    bool          isQuoted   = true;   // string is quoted
    int           indent     = 2;
    const char*   line_break = "\n";
    StringBuffer& os;
    bool          use_stringstream             = false;
    bool          clear_stream_before_printing = true;

    template <class T>
    static zeroerr::String type(const T& t) {
        return demangle(typeid(t).name());
    }

#if defined(ZEROERR_ENABLE_MAGIC_ENUM) && (ZEROERR_CXX_STANDARD >= 17)
    ZEROERR_ENABLE_IF(ZEROERR_IS_ENUM)
    print(const T& value, unsigned level, const char* lb, rank<0>) { os << tab(level) << magic_enum::enum_name(value) << lb; }
#else
    ZEROERR_ENABLE_IF((ZEROERR_IS_ENUM && detail::is_streamable<std::ostream, T>::value))
    print(const T& value, unsigned level, const char* lb, rank<0>) {
        os << tab(level) << value << lb;
    }

    ZEROERR_ENABLE_IF((ZEROERR_IS_ENUM && !detail::is_streamable<std::ostream, T>::value))
    print(const T& value, unsigned level, const char* lb, rank<0>) {
        os << tab(level) << "<unprintable enum " << type(value) << ">" << lb;
    }
#endif

    ZEROERR_ENABLE_IF(ZEROERR_IS_INT || ZEROERR_IS_FLOAT)
    print(const T& value, unsigned level, const char* lb, rank<0>) { os << tab(level) << value << lb; }

    ZEROERR_ENABLE_IF(ZEROERR_IS_POINTER &&
                      !std::is_function<typename std::remove_pointer<T>::type>::value)
    print(const T& value, unsigned level, const char* lb, rank<0>) {
        if (value == nullptr)
            os << tab(level) << "nullptr" << lb;
        else
            os << tab(level) << "<" << type(value) << " at " << static_cast<const void*>(value) << ">" << lb;
    }

    ZEROERR_ENABLE_IF(ZEROERR_IS_POINTER &&
                      std::is_function<typename std::remove_pointer<T>::type>::value)
    print(T value, unsigned level, const char* lb, rank<1>) {
        if (value == nullptr)
            os << tab(level) << "nullptr" << lb;
        else
            os << tab(level) << "<" << type(value) << " at "
               << reinterpret_cast<const void*>(value) << ">" << lb;
    }

    template <class R, class... Args>
    void print(const std::function<R(Args...)>& value, unsigned level, const char* lb, rank<3>) {
        if (!value)
            os << tab(level) << "nullptr" << lb;
        else
            os << tab(level) << "<" << type(value) << " holding "
               << demangle(value.target_type().name()) << ">" << lb;
    }


    // Preserve the pointer fallback for built-in arrays; character arrays use
    // the higher-priority string rule.
    template <typename T, std::size_t N>
    void print(const T (&value)[N], unsigned level, const char* lb, rank<0>) {
        print(value + 0, level, lb, rank<max_rank>{});
    }

    ZEROERR_ENABLE_IF(ZEROERR_IS_CLASS)
    print(const T& value, unsigned level, const char* lb, rank<0>) {
        os << tab(level) << type(value) << lb;
    }


    ZEROERR_ENABLE_IF(ZEROERR_IS_CHAR || ZEROERR_IS_WCHAR)
    print(const T& value, unsigned level, const char* lb, rank<1>) {
        os << tab(level) << '\'' << value << '\'' << lb;
    }

#if defined(ZEROERR_ENABLE_PFR) && (ZEROERR_CXX_STANDARD >= 14)
    template <class StructType, unsigned... I>
    void print_struct(const StructType& s, unsigned, const char*, detail::seq<I...>) {
        int _[] = {(os << (I == 0 ? "" : ", ") << pfr::get<I>(s), 0)...};
        (void)_;
    }

#if PFR_USE_CPP17 || PFR_USE_LOOPHOLE
    ZEROERR_ENABLE_IF(ZEROERR_IS_CLASS && ZEROERR_IS_POD)
#else
    ZEROERR_ENABLE_IF(detail::is_classic_pfr_printable<T>::value)
#endif
    print(const T& value, unsigned level, const char* lb, rank<1>) {
        os << tab(level) << "{";
        print_struct(value, level, isCompact ? " " : line_break,
                     detail::gen_seq<pfr::tuple_size<T>::value>{});
        os << tab(level) << "}" << lb;
    }
#endif

    ZEROERR_ENABLE_IF(ZEROERR_IS_BOOL)
    print(const T& value, unsigned level, const char* lb, rank<2>) {
        os << tab(level) << (value ? "true" : "false") << lb;
    }

    ZEROERR_ENABLE_IF(ZEROERR_IS_CLASS && ZEROERR_IS_STREAMABLE && !ZEROERR_IS_CONTAINER)
    print(const T& value, unsigned level, const char* lb, rank<2>) { os << tab(level) << value << lb; }


    ZEROERR_ENABLE_IF(ZEROERR_IS_CONTAINER)
    print(const T& value, unsigned level, const char* lb, rank<2>) {
        os << tab(level) << "{" << (isCompact ? "" : line_break);
        bool last = false;
        for (auto iter = value.begin(); iter != value.end(); ++iter) {
            auto next = iter;
            ++next;
            if (next == value.end()) last = true;
            print(*iter, level + 1, isCompact ? (last ? "" : ", ") : line_break, rank<max_rank>{});
        }
        os << tab(level) << "}" << lb;
    }

    ZEROERR_ENABLE_IF(ZEROERR_IS_CONTAINER && ZEROERR_IS_ARRAY)
    print(const T& value, unsigned level, const char* lb, rank<3>) {
        os << tab(level) << "[";
        bool last = false;
        for (auto iter = value.begin(); iter != value.end(); ++iter) {
            auto next = iter;
            ++next;
            if (next == value.end()) last = true;
            print(*iter, 0, last ? "" : ", ", rank<max_rank>{});
        }
        os << tab(level) << "]" << lb;
    }


    ZEROERR_ENABLE_IF(ZEROERR_IS_AUTOPTR && !ZEROERR_IS_CONTAINER)
    print(const T& value, unsigned level, const char* lb, rank<3>) {
        if (value.get() == nullptr)
            os << tab(level) << "nullptr" << lb;
        else
            os << tab(level) << "<" << type(value) << " at " << static_cast<const void*>(value.get()) << ">" << lb;
    }

    ZEROERR_ENABLE_IF(ZEROERR_IS_CONTAINER && ZEROERR_IS_MAP && !ZEROERR_IS_STRING &&
                      !ZEROERR_IS_COMPLEX && !detail::is_weak_pointer_like<T>::value)
    print(const T& value, unsigned level, const char* lb, rank<4>) {
        os << tab(level) << "{" << (isCompact ? "" : line_break);
        bool last = false;
        for (auto iter = value.begin(); iter != value.end(); ++iter) {
            auto next = iter;
            ++next;
            if (next == value.end()) last = true;
            print((*iter).first, level + 1, " : ", rank<max_rank>{});
            print((*iter).second, level + 1, isCompact ? (last ? "" : ", ") : line_break,
                  rank<max_rank>{});
        }
        os << tab(level) << "}" << lb;
    }

    ZEROERR_ENABLE_IF(detail::is_weak_pointer_like<T>::value && !ZEROERR_IS_STRING && !ZEROERR_IS_COMPLEX)
    print(const T& value, unsigned level, const char* lb, rank<4>) {
        const auto locked = value.lock();
        print(locked, level, lb, rank<3>{});
    }

    ZEROERR_ENABLE_IF(ZEROERR_IS_COMPLEX && !ZEROERR_IS_STRING)
    print(const T& value, unsigned level, const char* lb, rank<4>) {
        os << tab(level) << "(" << value.real() << "+" << value.imag() << "i)" << lb;
    }

    ZEROERR_ENABLE_IF(ZEROERR_IS_STRING)
    print(const T& value, unsigned level, const char* lb, rank<4>) {
        os << tab(level) << quote();
        write_string(value, typename detail::is_string_like<T>::type{});
        os << quote() << lb;
    }

    template <class T>
    void write_string(const T& value, std::true_type) {
        if (value.size() != 0)
            os.write(value.data(), static_cast<std::size_t>(value.size()));
    }
    template <class T>
    void write_string(const T& value, std::false_type) {
        const char* text = value;
        os << (text ? text : "nullptr");
    }

    template <class TupType>
    inline void print_tuple(const TupType&, unsigned, const char*, detail::seq<>) {}

    template <class TupType, unsigned... I>
    inline void print_tuple(const TupType& _tup, unsigned level, const char*, detail::seq<I...>) {
        int _[] = {(os << (I == 0 ? "" : ", "),
                    print(detail::tuple_get<I>(_tup), level + 1, "", rank<max_rank>{}), 0)...};
        (void)_;
    }

    ZEROERR_ENABLE_IF(detail::is_tuple_like<T>::value && !ZEROERR_IS_CONTAINER && !ZEROERR_IS_AUTOPTR)
    print(const T& value, unsigned level, const char* lb, rank<3>) {
        os << tab(level) << "(";
        print_tuple(value, level, isCompact ? " " : line_break, detail::gen_seq<std::tuple_size<T>::value>{});
        os << ")" << lb;
    }

    void print_text(const char* value) { os << value; }
    template <class T> void print_text(const T& value) { os.write(value.data(), value.size()); }
    zeroerr::String tab(unsigned level);
    const char* quote() { return isQuoted ? "\"" : ""; }

    static zeroerr::String demangle(const char* name);

    zeroerr::String str() const;
    operator zeroerr::String() const { return str(); }
    template <class T, typename std::enable_if<detail::StringLike<T>::owning &&
        !std::is_same<T, String>::value, int>::type = 0>
    operator T() const { return str(); }

};

inline StringBuffer& operator<<(StringBuffer& buffer, const Printer& printer) {
    if (printer.use_stringstream) buffer << printer.str();
    return buffer;
}

namespace detail {

// Only the concrete value dispatch is templated. Stream construction, printing
// order, formatting and cleanup all live in the shared implementation.
template <typename T>
void printDiagnosticValue(Printer& printer, const void* value) {
    printer.call(*static_cast<T*>(const_cast<void*>(value)));
}

template <typename T>
DiagnosticValue makeDiagnosticValue(T& value) {
    return {const_cast<const void*>(static_cast<const volatile void*>(std::addressof(value))),
            &printDiagnosticValue<T>};
}

}  // namespace detail

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
void PrinterExt(Printer& P, const T& v, unsigned level, const char* lb, rank<0>) {
    P.print(v, level, lb, rank<max_rank>{});
}

extern Printer& getStdoutPrinter();
extern Printer& getStderrPrinter();


}  // namespace zeroerr

ZEROERR_SUPPRESS_COMMON_WARNINGS_POP

#pragma pop_macro("_LIBCPP_REMOVE_TRANSITIVE_INCLUDES")
