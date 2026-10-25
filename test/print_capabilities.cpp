// Build this same test twice to exercise both public include orders.
#ifdef ZEROERR_TEST_STL_FIRST
#include "print_capabilities_stl.h"
#endif
#include "zeroerr/print.h"
#include "zeroerr/std_adapter.h"
#ifndef ZEROERR_TEST_STL_FIRST
#include "print_capabilities_stl.h"
#endif

namespace capabilities {
struct Complex {
    double real() const { return 2; }
    double imag() const { return 3; }
};
struct Pointer {
    const int* value;
    const int* get() const { return value; }
};
struct WeakPointer {
    Pointer lock() const { return Pointer{nullptr}; }
};
struct Text {
    using traits_type = std::char_traits<char>;
    const char* data() const { return "text"; }
    unsigned size() const { return 4; }
};
struct Range {
    int values[2] = {4, 5};
    const int* begin() const { return values; }
    const int* end() const { return values + 2; }
};
struct Entry { int first; int second; };
struct Map {
    Entry values[1] = {{1, 2}};
    const Entry* begin() const { return values; }
    const Entry* end() const { return values + 1; }
};
struct Tuple { int value; };
template <std::size_t I>
const int& get(const Tuple& value) {
    static_assert(I == 0, "only one element");
    return value.value;
}
struct NotPointer { int get() const; };
struct NotWeakPointer { int lock() const; };
struct MutableRange {
    int* begin();
    int* end();
};
struct Streamable {};
std::ostream& operator<<(std::ostream& os, const Streamable&) { return os << "stream"; }
struct StreamRange : Range {};
std::ostream& operator<<(std::ostream& os, const StreamRange&) { return os << "stream-range"; }
struct PointerRange : Range { const int* get() const { return nullptr; } };
struct Opaque;
struct Extended {};
void PrinterExt(zeroerr::Printer& p, const Extended&, unsigned, const char* lb, zeroerr::rank<5>) {
    p.os << "extension" << lb;
}
} // namespace capabilities

namespace std {
template <> struct tuple_size<capabilities::Tuple> : integral_constant<size_t, 1> {};
}

static_assert(zeroerr::detail::is_complex<capabilities::Complex>::value, "complex protocol");
static_assert(zeroerr::detail::is_string<capabilities::Text>::value, "text protocol");
static_assert(zeroerr::detail::is_tuple_like<capabilities::Tuple>::value, "ADL tuple protocol");
static_assert(!zeroerr::detail::is_pointer_like<capabilities::NotPointer>::value, "get must return pointer");
static_assert(!zeroerr::detail::is_weak_pointer_like<capabilities::NotWeakPointer>::value, "lock must return pointer wrapper");
static_assert(!zeroerr::detail::is_container<capabilities::MutableRange>::value, "printer requires const iteration");
static_assert(!zeroerr::detail::is_string<std::vector<char>>::value, "character buffers remain containers");

int main() {
    zeroerr::Printer p;
    p.isCompact = true;
    p.line_break = "";
    int failures = 0;
    auto expect = [&](const std::string& expected) {
        if (p.str() != expected) {
            std::cerr << "expected [" << expected << "], got [" << p.str() << "]\n";
            ++failures;
        }
    };
    p(1, true, 'x'); expect("1 true 'x'");
    p(nullptr); expect("nullptr");
    p("hello"); expect("\"hello\"");
    p(std::string("a\0b", 3)); expect(std::string("\"a\0b\"", 5));
#if ZEROERR_CXX_STANDARD >= 17
    p(std::string_view("a\0b", 3)); expect(std::string("\"a\0b\"", 5));
    p(std::string_view{}); expect("\"\"");
#endif
    p(std::complex<double>(2, 3)); expect("(2+3i)");
    p(capabilities::Complex{}); expect("(2+3i)");
    p(capabilities::Text{}); expect("\"text\"");
    p(capabilities::Range{}); expect("{4, 5}");
    p(capabilities::Map{}); expect("{1 : 2}");
    p(capabilities::Tuple{7}); expect("(7)");
    p(capabilities::Streamable{}); expect("stream");
    p(capabilities::StreamRange{}); expect("{4, 5}");
    p(capabilities::PointerRange{}); expect("{4, 5}");
    p(static_cast<capabilities::Opaque*>(nullptr)); expect("nullptr");
    int raw[] = {1, 2};
    p(raw);
    if (p.str().find(" at ") == std::string::npos) ++failures;
    p(capabilities::Extended{}); expect("extension");
    p(capabilities::Pointer{nullptr}); expect("nullptr");
    p(capabilities::WeakPointer{}); expect("nullptr");
    p(std::vector<int>{1, 2}); expect("[1, 2]");
    p(std::vector<char>{'a', 'b'}); expect("['a', 'b']");
    p(std::list<int>{1, 2}); expect("{1, 2}");
    p(std::array<int, 2>{{1, 2}}); expect("[1, 2]");
    p(std::map<int, std::vector<int>>{{1, {2, 3}}}); expect("{1 : [2, 3]}");
    p(std::tuple<int, const char*, std::complex<int>>{1, "two", {3, 4}});
    expect("(1, \"two\", (3+4i))");
    p(std::tuple<>{}); expect("()");
    p(std::pair<int, int>{1, 2}); expect("(1, 2)");
    p(std::vector<std::tuple<int>>{{1}, {2}}); expect("[(1), (2)]");
    auto tuple = std::make_tuple(1, 2);
    int visited = 0;
    zeroerr::detail::visit_at(tuple, 1, [&](int v) { visited = v; });
    if (visited != 2) ++failures;
    zeroerr::detail::visit2_at(tuple, tuple, 0, [&](int a, int b) { visited = a + b; });
    if (visited != 2) ++failures;
    std::unique_ptr<int> unique(new int(42));
    p(unique);
    if (!unique || p.str().find(" at ") == std::string::npos) ++failures;
    p(std::move(unique));
    if (!unique) ++failures; // Printing must not take ownership.
    p(std::unique_ptr<int>{}); expect("nullptr");
    auto shared = std::make_shared<int>(42);
    std::weak_ptr<int> weak = shared;
    p(weak);
    if (p.str().find(" at ") == std::string::npos || shared.use_count() != 1) ++failures;
    shared.reset();
    p(weak); expect("nullptr");
    p.isCompact = false;
    p(std::vector<std::vector<int>>{{1, 2}}); expect("[[1, 2]]");
    std::ostringstream os;
    zeroerr::Printer external(os);
    external(42);
    if (os.str() != "42\n") ++failures;
    try { external.str(); ++failures; } catch (const std::runtime_error&) {}
    return failures ? 1 : 0;
}
