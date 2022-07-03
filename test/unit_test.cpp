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