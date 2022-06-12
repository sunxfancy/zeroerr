#include "zeroerr/unittest.h"
#include "zeroerr/assert.h"
#include "zeroerr/color.h"

#include <iostream>

namespace zeroerr {


UnitTest& UnitTest::parseArgs(int argc, char** argv) { return *this; }

int UnitTest::run() {
    std::cout << "ZeroErr Unit Test";
    TestContext context;
    unsigned    passed = 0, warning = 0, failed = 0, skipped = 0;

    for (auto& tc : detail::getRegisteredTests()) {
        std::cout << std::endl
                  << "TEST CASE " << FgCyan << tc.name << Reset << Dim << " [" << tc.file << ":"
                  << tc.line << "]" << Reset << std::endl
                  << std::endl;
        try {
            tc.func(&context);  // run the test case
        } catch (const AssertionData& e) {
            failed++;
            continue;
        } catch (const std::exception& e) {
            failed++;
            continue;
        }
        passed++;
    }
    std::cout << "----------------------------------------------------------------" << std::endl;
    std::cout << "             " << FgGreen << "PASSED" << Reset << "   |   " << FgYellow
              << "WARNING" << Reset << "   |   " << FgRed << "FAILED" << Reset << "   |   " << Dim
              << "SKIPPED" << Reset << std::endl;
    std::cout << "TEST CASE: " << std::endl;
    std::cout << "ASSERTION: " << std::endl;
    return 0;
}


bool TestCase::operator<(const TestCase& rhs) const {
    return (file < rhs.file) || (file == rhs.file && line < rhs.line);
}

namespace detail {

std::set<TestCase>& getRegisteredTests() {
    static std::set<TestCase> data;
    return data;
}

}  // namespace detail

}  // namespace zeroerr


int main(int argc, char** argv) { return zeroerr::UnitTest().parseArgs(argc, argv).run(); }
