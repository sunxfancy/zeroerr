#include "zeroerr/assert.h"
#include "zeroerr/unittest.h"
#include <stdexcept>

int coverage_subject(int);
bool coverage_condition(bool, bool);

TEST_CASE("positive \"quoted\"") { CHECK(coverage_subject(1) == 10); }
TEST_CASE("positive duplicate") { CHECK(coverage_subject(2) == 10); }
TEST_CASE("negative") { CHECK(coverage_subject(-1) == -10); }
TEST_CASE("nested") {
    SUB_CASE("positive") { CHECK(coverage_subject(1) == 10); };
    SUB_CASE("negative") { CHECK(coverage_subject(-1) == -10); };
}
TEST_CASE("condition true") { CHECK(coverage_condition(true, true)); }
TEST_CASE("condition false") { CHECK(!coverage_condition(true, false)); }
TEST_CASE("condition short circuit") { CHECK(!coverage_condition(false, false)); }
TEST_CASE("exception") {
    coverage_subject(-1);
    throw std::runtime_error("intentional fixture failure");
}
TEST_CASE("skipped", zeroerr::skip()) { CHECK(coverage_subject(0) == 0); }
