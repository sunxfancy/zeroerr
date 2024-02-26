#define ZEROERR_ENABLE_PFR
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
    std::cerr << "a = 100" << std::endl;

    ZEROERR_HAVE_SAME_OUTPUT;
}


TEST_CASE("traditional check macro") {
    int a = 1;
    int b = 2;
    CHECK_EQ(a, b);
}

TEST_CASE("parsing arguments") {
    int argc = 2;
    const char* argvs[4][2] = {
        {"test", "-v"},
        {"test", "-q"},
        {"test", "--verbose"},
        {"test", "--quiet"},
    };

    zeroerr::UnitTest ut;

    ut.parseArgs(argc, argvs[0]);
    CHECK_EQ(ut.silent, false);

    ut.parseArgs(argc, argvs[1]);
    CHECK_EQ(ut.silent, true);

    ut.parseArgs(argc, argvs[2]);
    CHECK_EQ(ut.silent, false);

    ut.parseArgs(argc, argvs[3]);
    CHECK_EQ(ut.silent, true);
}

struct Node
{
    int val;
    const Node* next;
    bool operator==(const Node& other) const {
        return val == other.val && next == other.next;
    }
};

TEST_CASE("assertion") {
    // reference 

    int a = 1;
    const int &b = a;

    CHECK(b == 1);

    const Node n1{1, nullptr};
    Node n2{2, &n1};

    CHECK(n2 == n1);

    CHECK(n2.val == 0 AND n1.val == 0);
}

TEST_CASE("range") {
    float a = 2.5;
    CHECK(0.5 <= a < 3.0);
    CHECK(0.5 <= a <= 1.0);
}

static int targetFunc(int a, int b) {
    if (a == 3 && b == 5) return 7;
    return a + b;
}


TEST_CASE("combinational test args") {
    TestArgs<int> a{1, 2, 3};
    TestArgs<int> b{4, 5, 6};

    CombinationalTest test([&]{
        CHECK(targetFunc(a, b) == (a+b));
    });
    test(a, b);
}

