#define ZEROERR_IMPLEMENTATION
#include "zeroerr.hpp"

using namespace zeroerr;


TEST_CASE("Try logging") {
    std::vector<int> data = {1, 2, 3};
    LOG_IF(1 == 1, "data = {data}", data);
}

// -------------------------------

struct Node {
    std::string name;
    int id;
};

std::ostream& operator<<(std::ostream& out, Node n) {
    out << n.id << '.' << n.name;
    return out;
}

TEST_CASE("Try logging with custom type") {
    std::map<std::string, std::vector<Node>> data = {
        {"user1", {{"a",1}, {"b",2}}}, {"user2", {{"c",3}, {"d",4}}}
    };
    LOG("data = {data}", data);
}

// -------------------------------


int fib(int n) {
    REQUIRE(n >= 0, "n must be non-negative");
    REQUIRE(n < 20, "n must be less than 20");
    if (n <= 2) {
        return 1;
    }
    return fib(n - 1) + fib(n - 2);
}

TEST_CASE("fib function test") {
    CHECK(fib(0) == 0);
    CHECK(fib(1) == 1);
    CHECK(fib(2) == 1);
    CHECK(fib(3) == 2);
    CHECK(fib(4) == 3);
    CHECK(fib(5) == 5);
    CHECK_THROWS(fib(20));
}
