#include <iomanip>
#include "zeroerr/std_adapter.h"
#include <limits>
#include <sstream>
#include <vector>
#include "zeroerr/assert.h"
#include "zeroerr/format.h"
#include "zeroerr/unittest.h"

using namespace zeroerr;

namespace {
struct StreamValue {
    int value;
};
std::ostream& operator<<(std::ostream& os, StreamValue v) {
    return os << '[' << std::hex << v.value << ']';
}
struct ExtensionValue {
    int value;
};
void PrinterExt(Printer& p, ExtensionValue v, unsigned, const char* lb, rank<5>) {
    p.os << "extension:" << v.value << lb;
}
struct AddressValue {
    int           value;
    AddressValue* operator&() { throw std::runtime_error("overloaded address-of called"); }
};
std::ostream& operator<<(std::ostream& os, const AddressValue& v) { return os << v.value; }
bool          operator==(const AddressValue& v, int rhs) { return v.value == rhs; }
struct ObservedValue {
    int* order;
    int  value;
};
std::ostream& operator<<(std::ostream& os, const ObservedValue& v) {
    *v.order = *v.order * 10 + v.value;
    return os << v.value;
}
void requireAt(bool condition, unsigned line) {
    if (!condition)
        throw std::runtime_error("diagnostic text regression at line " + std::to_string(line));
}
#define require(condition) requireAt((condition), __LINE__)
}  // namespace

TEST_CASE("text buffer growth and embedded NUL") {
    detail::TextBuffer buffer;
    require(buffer.size() == 0 && buffer.data()[0] == '\0');
    buffer.append(nullptr, 0);
    std::string expected(255, 'x');
    buffer.append(expected.data(), expected.size());
    require(buffer.str() == expected);
    buffer.append('\0');
    expected.push_back('\0');
    require(buffer.str() == expected && buffer.data()[buffer.size()] == '\0');
    for (int i = 0; i < 10000; ++i) {
        buffer.append('y');
        expected.push_back('y');
    }
    require(buffer.str() == expected);
    bool overflow = false;
    try {
        buffer.append("", (std::numeric_limits<std::size_t>::max)());
    } catch (const std::length_error&) {
        overflow = true;
    }
    require(overflow && buffer.str() == expected);
}

TEST_CASE("text buffer self append with reallocation") {
    detail::TextBuffer buffer;
    const std::string  initial(200, 'a');
    buffer.append(initial.data(), initial.size());
    buffer.append(buffer.data(), buffer.size());
    require(buffer.str() == initial + initial);
    buffer.append(buffer.data() + 100, 20);
    require(buffer.str() == std::string(420, 'a'));
}

TEST_CASE("text stream preserves ostream formatting") {
    detail::TextStream actual;
    std::stringstream  expected;
    actual.stream << std::hex << std::setw(6) << std::setfill('0') << 42 << ' ' << std::fixed
                  << std::setprecision(3) << 1.25 << '\0' << StreamValue{255};
    expected << std::hex << std::setw(6) << std::setfill('0') << 42 << ' ' << std::fixed
             << std::setprecision(3) << 1.25 << '\0' << StreamValue{255};
    actual.stream.flush();
    require(actual.stream.str() == expected.str());
}

TEST_CASE("format compatibility and bounds") {
    require(format("plain") == "plain");
    require(format("") == "");
    require(format("a{name}b{}c", "x", 42) == "axb42c");
    require(format("{}{}", "", "x") == "x");
    require(format("{}", std::string("a\0b", 3)) == std::string("a\0b", 3));
    require(format("{}", std::string(4096, 'x')) == std::string(4096, 'x'));
    require(format("{}", std::vector<int>{1, 2}) == "[1, 2]");
    require(format("{} {}", StreamValue{255}, 16) == "[ff] 10");
    require(format("{}", ExtensionValue{7}) == "extension:7");
    require(format("only {}", 1, 2) == "only 1");
    require(format("before {unfinished", 1) == "before ");
    require(format("a{{name}b", 1) == "a1b");
    bool rejected = false;
    try {
        format("{} {}", 1);
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    require(rejected);
    rejected = false;
    try {
        format("{}");
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    require(rejected);
}

TEST_CASE("decomposition comparison chains and logical joins") {
    int        a = 1, b = 2;
    ExprResult lvalue = ExpressionDecomposer() << a == b;
    ExprResult rvalue = ExpressionDecomposer() << a == 1;
    ExprResult chain  = ExpressionDecomposer() << a < b < 3;
    ExprResult single = ExpressionDecomposer() << true;
    require(!lvalue.res && lvalue.decomp == "1 == 2");
    require(rvalue.res && rvalue.decomp == "1 == 1");
    require(chain.res && chain.decomp == "1 < 2 < 3");
    require(single.res && single.decomp == "true");
    ExprResult both   = rvalue && lvalue;
    ExprResult either = lvalue || rvalue;
    require(!both.res && both.decomp == "1 == 1 && 1 == 2");
    require(either.res && either.decomp == "1 == 2 || 1 == 1");
    const std::string long_text(4096, 'a');
    ExprResult        strings = ExpressionDecomposer() << long_text == long_text;
    require(strings.res && strings.decomp == '"' + long_text + "\" == \"" + long_text + '"');
}

TEST_CASE("public printer and assertion text remain compatible") {
    Printer p;
    p.isCompact  = true;
    p.line_break = "";
    require(p(1).str() == "1");
    require(p(2).str() == "2");
    p.clear_stream_before_printing = false;
    require(p(3).str() == "23");
    std::stringstream stream;
    Printer           external(stream);
    external(1);
    require(stream.str() == "1\n");
    const std::string condition("a\0b", 3), message("c\0d", 3);
    std::stringstream expected;
    expected << "    " << condition << "  expands to  " << message << std::endl;
    expected << Dim << "(" << "test.cpp" << ":" << 123 << ")" << Reset << std::endl;
    require(detail::assertionText(condition, message, "test.cpp", 123) == expected.str());
}

TEST_CASE("diagnostic callbacks preserve addresses qualifiers and evaluation order") {
    AddressValue address{7};
    require(format("{}", address) == "7");
    ExprResult custom = ExpressionDecomposer() << address == 7;
    require(custom.res && custom.decomp == "7 == 7");
    volatile int number    = 3;
    ExprResult   qualified = ExpressionDecomposer() << number == 3;
    require(qualified.res && qualified.decomp == "3 == 3");
    int order = 0;
    require(format("{} {}", ObservedValue{&order, 1}, ObservedValue{&order, 2},
                   ObservedValue{&order, 3}) == "1 2");
    require(order == 123);  // Surplus arguments still print in argument order.
}
