#include <zeroerr/unittest.h>
int main(int argc, char** argv) { return zeroerr::UnitTest().parseArgs(argc, const_cast<const char**>(argv)).run(); }
