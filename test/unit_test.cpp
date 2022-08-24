#include "zeroerr/assert.h"
#include "zeroerr/dbg.h"
#include "zeroerr/print.h"
#include "zeroerr/unittest.h"


using namespace zeroerr;

class A {
public:
    int add(int a, int b) {
        k = a + b;
        return k;
    }
    int k = 0;
};

void B(A* a, int b) {
    if (b == 5) return;
    if (a->k != b) a->k = b;
}


TestedObjects<A> collectA;


TEST_CASE("test class A") {
    A a;
    CHECK(a.add(1, 2) == 3);

    collectA.add(std::move(a));
}


TEST_CASE("test function B") {
    std::cerr << "sizeof(A): " << collectA.objects.size() << std::endl;
    for (auto& a : collectA.objects) {
        for (int b : {1, 2, 5}) {
            A a_copy = a;
            B(&a_copy, b);
            CHECK(a_copy.k == b);
        }
    }
}


TEST_CASE("assert match") {
    auto p = start_with("test");
    std::cerr << "🏁 p: " << p->match("test your string") << std::endl;
    CHECK(start_with("test")->match("test your string"));
}

TEST_CASE("assert combine matcher") {
    auto p = start_with("test") || start_with("check");

    dbg(p->match("test your string"));
    dbg(p->match("check your string"));
}


TEST_CASE("assert in if") {
    int a = 0;
    if (CHECK(a == 1)) {
        std::cerr << "a == 1" << std::endl;
    }
}


class test_fixture {
public:
    test_fixture() { std::cerr << "test_fixture()" << std::endl; }
    ~test_fixture() { std::cerr << "~test_fixture()" << std::endl; }

    int getK() { return 1; }
};

TEST_CASE_FIXTURE(test_fixture, "test fixture") { dbg(getK()); }


TEST_CASE("test sub cases") {
    SUB_CASE("sub case 1") { CHECK(1 == 1); };
    SUB_CASE("sub case 2") { CHECK(1 == 2); };
    SUB_CASE("sub case 3") { CHECK(2 == 2); };
}


TEST_CASE("match ostream") {
    // match output can be done in the following workflow
    // 1. user mark the test case which are comparing output use 'have_same_output'
    // 2. If the output is not exist, the result has been used as a correct verifier.
    // 3. If the output is exist, compare with it and report error if output is not match.
    int a = rand();
    std::cerr << "a = " << a << std::endl;

    ZEROERR_HAVE_SAME_OUTPUT;
}