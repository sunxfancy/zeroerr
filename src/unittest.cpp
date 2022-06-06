#include "zeroerr/unittest.h"
#include <iostream>

namespace zeroerr
{
    

UnitTest& UnitTest::parseArgs(int argc, char** argv) {
    return *this;
}

int UnitTest::run() {
    for (auto& tc : detail::getRegisteredTests()) {
        std::cout << "[" << tc.file << ":" << tc.line << "] " << tc.name << std::endl;
        tc.func(); // run the test case
    }
    return 0;
}


bool TestCase::operator<(const TestCase& rhs) const {
    return (file < rhs.file) || (file == rhs.file && line < rhs.line);
}

namespace detail
{
    
std::set<TestCase>& getRegisteredTests() {
    static std::set<TestCase> data;
    return data;
}

} // namespace detail

} // namespace zeroerr


int main(int argc, char** argv) { 
    return zeroerr::UnitTest().parseArgs(argc, argv).run(); 
}


