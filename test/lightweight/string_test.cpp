#include "zeroerr/string.h"

#include "forbidden_headers.h"

#include <stdexcept>
#include <string>

using zeroerr::String;

int main() {
    String empty;
    if (!empty.empty() || empty.data()[0] != '\0') return 1;
    empty.append(nullptr, 0);
    String small("abc");
    small.append(small.data(), small.size());
    if (small != "abcabc") return 2;
    String large(1000, 'x');
    large.append(large.data(), large.size());
    if (large != String(2000, 'x')) return 3;
    String copy(large);
    copy[0] = 'y';
    if (large[0] != 'x' || copy[0] != 'y') return 4;
    String moved(static_cast<String&&>(copy));
    if (!copy.empty() || moved.size() != 2000 || moved[0] != 'y') return 5;
    moved = static_cast<String&&>(moved);
    if (moved.size() != 2000) return 6;
    moved = static_cast<String&&>(small);
    if (moved != "abcabc" || !small.empty()) return 7;
    String binary("a\0b", 3);
    if (binary.size() != 3 || binary == "a" || binary.substr(1, 2) != String("\0b", 2)) return 8;
    if (binary.find('\0') != 1 || binary.find(String("\0b", 2)) != 1) return 9;
    if (binary.find("", 3) != 3 || binary.find("", 4) != String::npos) return 10;
    String alias("0123456789");
    alias.replace(2, 4, alias);
    if (alias != "0101234567896789") return 11;
    alias = alias.c_str() + 2;
    if (alias != "01234567896789") return 12;
    alias.resize(3);
    alias.resize(7, 'x');
    if (alias != "012xxxx" || alias.find_last_of('x') != 6) return 13;
    if (String("a") + "b" != "ab" || "a" + String("b") != "ab") return 14;
    bool bounds = false, overflow = false;
    try {
        binary.substr(4);
    } catch (const std::out_of_range&) {
        bounds = true;
    }
    try {
        binary.append("", String::npos);
    } catch (const std::length_error&) {
        overflow = true;
    }
    if (!bounds || !overflow || binary != String("a\0b", 3)) return 15;
    // Differential testing against a deliberately included reference string.
    unsigned    state = 0x12345678;
    String      actual;
    std::string expected;
    for (unsigned step = 0; step < 4000; ++step) {
        state              = state * 1664525u + 1013904223u;
        unsigned operation = state % 6;
        char     bytes[8];
        for (unsigned i = 0; i < 8; ++i) bytes[i] = static_cast<char>((state >> (i * 3)) & 7);
        std::size_t count = (state >> 8) % 9;
        std::size_t pos   = expected.empty() ? 0 : (state >> 12) % (expected.size() + 1);
        if (operation == 0) {
            actual.append(bytes, count);
            expected.append(bytes, count);
        }
        if (operation == 1) {
            actual.replace(pos, count, String(bytes, count));
            expected.replace(pos, count, std::string(bytes, count));
        }
        if (operation == 2 && !expected.empty()) {
            count = count < expected.size() - pos ? count : expected.size() - pos;
            actual.append(actual.data() + pos, count);
            expected.append(expected.data() + pos, count);
        }
        if (operation == 3) {
            actual.resize((state >> 10) % 48, '\0');
            expected.resize((state >> 10) % 48, '\0');
        }
        if (operation == 4) {
            String duplicate(actual);
            actual = static_cast<String&&>(duplicate);
        }
        if (operation == 5 && expected.size() > 100) {
            actual.clear();
            expected.clear();
        }
        if (actual.size() != expected.size()) return 16;
        for (std::size_t i = 0; i < actual.size(); ++i)
            if (actual[i] != expected[i]) return 17;
        if (actual.data()[actual.size()] != '\0') return 18;
        if (actual.find(String(bytes, count)) != expected.find(std::string(bytes, count)))
            return 19;
        if (actual.rfind(String(bytes, count)) != expected.rfind(std::string(bytes, count)))
            return 20;
        if (actual.find_last_of('\0') != expected.find_last_of('\0')) return 21;
    }
    return 0;
}
