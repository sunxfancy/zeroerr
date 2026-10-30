#include <zeroerr/assert.h>
#include <zeroerr/unittest.h>
#include <stdexcept>
#include <memory>
#include <string>

namespace {
void helper() { CHECK(false); }
void invariant() { ASSERT(false); }
void requirement() { REQUIRE(false); }
}  // namespace

TEST_CASE("assertion.singleEvaluation") {
    int left = 0, right = 0;
    CHECK_EQ(++left, ++right);
    REQUIRE_EQ(left, 1);
    REQUIRE_EQ(right, 1);
    CHECK_EQ(std::string("value"), std::string("value"));
    CHECK_NE(left, right + 1);
    CHECK_LT(left, right + 1);
    CHECK_LE(left, right);
    CHECK_GT(left + 1, right);
    CHECK_GE(left, right);
    const int& reference = left;
    CHECK_EQ(reference, 1);
    std::unique_ptr<int> pointer(new int(7));
    CHECK_EQ(std::move(pointer), pointer);
    REQUIRE(pointer != nullptr);
    CHECK_EQ("value", std::string("value"));
    // The right operand must be typed in the caller's scope.
    double zeroerr_lhs = 1.5;
    CHECK_LT(1, zeroerr_lhs);
    CHECK_EQ([] { return 3; }(), 3);
    CHECK_EQ(0, 0);
    CHECK_EQ(false, false);
}
TEST_CASE("assertion.check") { CHECK(false); }
TEST_CASE("assertion.comparison") { CHECK_EQ(1, 2); }
TEST_CASE("assertion.checkNot") { CHECK_NOT(true); }
TEST_CASE("assertion.checkThrows") { CHECK_THROWS(false); }
TEST_CASE("assertion.require") { REQUIRE(false); }
TEST_CASE("assertion.assert") { ASSERT(false); }
TEST_CASE("assertion.helper") { helper(); }
TEST_CASE("assertion.helperRequire") { requirement(); }
TEST_CASE("assertion.helperAssert") { invariant(); }
TEST_CASE("assertion.subcaseHelper") {
    SUB_CASE("helper assertion") { helper(); };
}
TEST_CASE("assertion.expectedInvariant") { CHECK_THROWS((invariant(), false)); }

#ifdef ZEROERR_ASSERTION_CONTRACT_MAIN
int main(int argc, char** argv) {
    return zeroerr::UnitTest().parseArgs(argc, const_cast<const char**>(argv)).run();
}
#endif
