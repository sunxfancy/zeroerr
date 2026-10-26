#pragma once
#include <memory>
#include <sstream>
using namespace zeroerr;

TEST_CASE("smart character pointers print addresses without reading pointees") {
    auto check_address = [&](const auto& pointer) {
        Printer print;
        print(pointer);
        std::ostringstream expected;
        expected << "<" << Printer::type(pointer) << " at "
                 << static_cast<const void*>(pointer.get()) << ">\n";
        CHECK(print.str() == expected.str().c_str());
    };

    std::unique_ptr<char[]> restored(new char[3]{'a', 'b', 'c'});
    REQUIRE(restored != nullptr);
    check_address(restored);
    std::unique_ptr<char> scalar(new char('x'));
    check_address(scalar);
    std::shared_ptr<char> shared(new char('y'));
    check_address(shared);
    std::unique_ptr<unsigned char[]> bytes(new unsigned char[2]{1, 2});
    check_address(bytes);
    std::unique_ptr<const char[]> constant(new const char[2]{'d', 'e'});
    check_address(constant);

    restored.reset();
    REQUIRE(restored == nullptr);
    Printer print;
    print(restored);
    CHECK(print.str() == "nullptr\n");
}
