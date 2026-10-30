#define ZEROERR_ENABLE_PFR


#include "zeroerr/std_adapter.h"
#include "zeroerr/print.h"
#include "zeroerr/assert.h"
#include "zeroerr/dbg.h"
#include "zeroerr/unittest.h"

#include <complex>
#include <iostream>
#include <map>
#include <memory>
#include <tuple>
#include <vector>


using namespace zeroerr;

enum class UnstreamableState { Ready };

TEST_CASE("unstreamable enum has a diagnostic fallback") {
    std::stringstream output;
    Printer           print(output);
    print(UnstreamableState::Ready);
    CHECK(output.str().find("<unprintable enum ") != std::string::npos);
    CHECK(output.str().find("UnstreamableState") != std::string::npos);
}

TEST_CASE("print_test") {
    if (detail::is_streamable<std::ostream, int>::value) {
        std::cerr << "int is streamable" << std::endl;
    } else {
        std::cerr << "int is not streamable" << std::endl;
    }

    std::cerr << "Print Test Start!" << std::endl;

    auto print = getStderrPrinter();
    print(1, 2, 3);
    int  a = 1;
    int& b = a;
    int* c = &a;
    print(a, b, c);

    std::vector<int> vec = {1, 5, 2};
    print(vec, "hello", dbg(std::complex<float>(1, 2.54f)));

    auto& myvec = vec;
    print(myvec);

    auto get_student = [](int id) {
        if (id == 0) return std::make_tuple(3.8, 'A', "Lisa Simpson");
        if (id == 1) return std::make_tuple(2.9, 'C', "Milhouse Van Houten");
        if (id == 2) return std::make_tuple(1.7, 'D', "Ralph Wiggum");
        throw std::invalid_argument("id");
    };
    print({get_student(0), get_student(1), get_student(2)});

    std::shared_ptr<int> p(new int(42));
    print(p);

    std::unique_ptr<int> up(new int(42));
    print(std::move(up));

    std::vector<std::map<std::string, int>> foo{{{"a", 1}, {"b", 2}}, {{"c", 3}, {"d", 4}}};
    print(foo);

    std::map<std::string, int> bar{{"a", 1}, {"b", 2}};

    if (zeroerr::detail::ele_type_is_pair<decltype(bar)>::value) {
        std::cerr << "bar is a pair" << std::endl;
    } else {
        std::cerr << "bar is not a pair" << std::endl;
    }

    CHECK(a == 2);

    int k = dbg(1, 2);
    CHECK(k == 2);
}


struct A {
    std::string name;
    int         age;
};


TEST_CASE("print test with PFR library") {
    A    a{"John", 20};
    auto print = getStderrPrinter();

    std::cerr << "Is trivial:" << std::is_trivial<A>::value << std::endl;
    std::cerr << "Is standard layout:" << std::is_standard_layout<A>::value << std::endl;
    dbg(a);
}

TEST_CASE("PFR respects the selected reflection backend") {
    Printer print;
    print.isCompact = true;
    A value{"John", 20};
    print(value);
#if ZEROERR_CXX_STANDARD >= 14 && (PFR_USE_CPP17 || PFR_USE_LOOPHOLE)
    REQUIRE(print.str().find("John") != zeroerr::String::npos);
#else
    REQUIRE(print.str() == Printer::type(value) + "\n");
#endif
#if ZEROERR_CXX_STANDARD >= 14
    struct FlatPod { int first; double second; };
    print(FlatPod{1, 2});
    REQUIRE(print.str() == "{1, 2}\n");
#if !PFR_USE_CPP17 && !PFR_USE_LOOPHOLE
    struct LinkedPod { int value; const LinkedPod* next; };
    LinkedPod linked{1, nullptr};
    print(linked);
    REQUIRE(print.str() == Printer::type(linked) + "\n");
#endif
#endif
}

void outside_of_test() {
    int k = 0;
    CHECK(k != 0);
}

constexpr zeroerr::TestContext* test_expr = nullptr;

TEST_CASE("outside of the test") {
    auto print = getStderrPrinter();
    outside_of_test();
}

TEST_CASE("print in stream") {
    std::map<std::string, int> bar{{"a", 1}, {"b", 2}};

    Printer print;
    print.isCompact = true;
    std::cerr << "map: " << print(bar) << std::endl;
}

namespace {
int printable_function(int value) { return value; }
void printable_variadic_function(const char*, ...) {}
void printable_noexcept_function() noexcept {}
}

TEST_CASE("print function pointers") {
    Printer print;
    auto pointer = &printable_function;
    std::ostringstream expected;
    expected << "<" << Printer::type(pointer) << " at "
             << reinterpret_cast<const void*>(pointer) << ">\n";
    CHECK(print(pointer).str() == expected.str());
    CHECK(print(printable_function).str() == expected.str());
    CHECK(print(static_cast<decltype(pointer)>(nullptr)).str() == "nullptr\n");
    CHECK(print(&printable_variadic_function).str().find(" at ") != std::string::npos);
    CHECK(print(&printable_noexcept_function).str().find(" at ") != std::string::npos);

    print.isCompact = true;
    std::vector<decltype(pointer)> pointers{pointer, nullptr};
    CHECK(print(pointers).str() == "[" + expected.str().substr(0, expected.str().size() - 1) +
                                     ", nullptr]\n");
}

TEST_CASE("print std function") {
    Printer print;
    std::function<int(int)> empty;
    CHECK(print(empty).str() == "nullptr\n");
    std::function<int(int)> null_pointer = static_cast<int (*)(int)>(nullptr);
    CHECK(print(null_pointer).str() == "nullptr\n");

    std::function<int(int)> function = printable_function;
    const std::string expected = "<" + Printer::type(function) + " holding " +
                                 Printer::demangle(function.target_type().name()) + ">";
    CHECK(print(function).str() == expected + "\n");

    int calls = 0;
    auto lambda = [&calls](int value) { ++calls; return value; };
    function = lambda;
    CHECK(print(function).str() == "<" + Printer::type(function) + " holding " +
                                      Printer::type(lambda) + ">\n");
    CHECK(calls == 0);

    print.isCompact = true;
    std::vector<std::function<int(int)>> functions{printable_function, empty};
    CHECK(print(functions).str() == "[" + expected + ", nullptr]\n");
}

#include "smart_character_pointer_cases.h"
