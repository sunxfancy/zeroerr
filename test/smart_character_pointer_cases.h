#pragma once
#include <memory>
#include <sstream>
using namespace zeroerr;

template <class Pointer>
static bool smartCharacterPointerAddressMatches(const Pointer& pointer) {
    Printer print;
    print(pointer);
    std::ostringstream expected;
    expected << "<" << Printer::type(pointer) << " at "
             << static_cast<const void*>(pointer.get()) << ">\n";
    return print.str() == expected.str().c_str();
}

TEST_CASE("smart character pointers print addresses without reading pointees") {
    std::unique_ptr<char[]> restored(new char[3]{'a', 'b', 'c'});
    REQUIRE(restored != nullptr);
    CHECK(smartCharacterPointerAddressMatches(restored));
    std::unique_ptr<char> scalar(new char('x'));
    CHECK(smartCharacterPointerAddressMatches(scalar));
    std::shared_ptr<char> shared(new char('y'));
    CHECK(smartCharacterPointerAddressMatches(shared));
    std::unique_ptr<unsigned char[]> bytes(new unsigned char[2]{1, 2});
    CHECK(smartCharacterPointerAddressMatches(bytes));
    std::unique_ptr<const char[]> constant(new const char[2]{'d', 'e'});
    CHECK(smartCharacterPointerAddressMatches(constant));

    restored.reset();
    REQUIRE(restored == nullptr);
    Printer print;
    print(restored);
    CHECK(print.str() == "nullptr\n");
}
