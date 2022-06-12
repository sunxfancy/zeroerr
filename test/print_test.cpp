#define ZEROERR_ENABLE_PFR
#include "zeroerr/print.h"
#include "zeroerr/assert.h"
#include "zeroerr/dbg.h"
#include "zeroerr/unittest.h"

#include <iostream>
#include <map>
#include <vector>

using namespace zeroerr;

TEST_CASE("print_test") {
    if (detail::is_streamable<std::ostream, int>::value) {
        std::cout << "int is streamable" << std::endl;
    } else {
        std::cout << "int is not streamable" << std::endl;
    }

    std::cout << "Print Test Start!" << std::endl;

    auto print = getStderrPrinter();
    print(1, 2, 3);
    int  a = 1;
    int& b = a;
    int* c = &a;
    print(a, b, c);

    std::vector<int> vec = {1, 5, 2};
    print(vec, "hello", dbg(std::complex<float>(1, 2.54)));

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
        std::cout << "bar is a pair" << std::endl;
    } else {
        std::cout << "bar is not a pair" << std::endl;
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