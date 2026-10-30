#ifdef ZEROERR_REGRESSION_SINGLE_HEADER
#define ZEROERR_NO_MAIN
#define ZEROERR_DISABLE_AUTO_INIT
#define ZEROERR_IMPLEMENTATION
#define ZEROERR_ENABLE_FUZZTEST
#include "zeroerr.hpp"
#else
#include "zeroerr/domains/container_of.h"
#include "zeroerr/domains/element_of.h"
#include "zeroerr/domains/in_range.h"
#endif

#include <cmath>
#include <cstdio>
#include <limits>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

static void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

template <class F>
static void rejects(F action) {
    try { action(); }
    catch (const std::invalid_argument&) { return; }
    throw std::runtime_error("Expected invalid_argument");
}

template <class Container>
static void containers() {
    zeroerr::Rng rng(42);
    auto domain = zeroerr::ContainerOf<Container>(zeroerr::InRange<int>(0, 100));
    domain.WithSize(0);
    auto empty = domain.GetRandomCorpus(rng);
    require(empty.empty(), "Fixed empty corpus");
    for (int i = 0; i < 100; ++i) domain.Mutate(rng, empty, i % 2 == 0);
    require(empty.empty(), "Empty mutation must stay empty");
    domain.WithSize(7);
    auto fixed = domain.GetRandomCorpus(rng);
    for (int i = 0; i < 100; ++i) {
        require(fixed.size() == 7, "Fixed corpus length");
        domain.Mutate(rng, fixed, i % 2 == 0);
    }
    auto variable = zeroerr::ContainerOf<Container>(zeroerr::InRange<int>(0, 100));
    variable.WithMinSize(0);
    variable.WithMaxSize(10);
    auto corpus = variable.GetRandomCorpus(rng);
    for (int i = 0; i < 1000; ++i) {
        const auto before = corpus.size();
        variable.Mutate(rng, corpus, true);
        require(corpus.size() <= before, "Shrink must not grow corpus");
        variable.Mutate(rng, corpus, false);
        require(corpus.size() <= 10, "Maximum corpus length");
    }
    variable.WithMinSize(11);
    rejects([&] { variable.GetRandomCorpus(rng); });
    rejects([&] { variable.WithSize((std::numeric_limits<unsigned>::max)()); });
}

static void ranges() {
    zeroerr::Rng rng(42);
    zeroerr::InRange<double> real(0.0, 0.5);
    bool interior = false;
    for (int i = 0; i < 100; ++i) {
        double value = real.GetRandomCorpus(rng);
        require(value >= 0 && value <= 0.5, "Floating bounds");
        interior |= value > 0 && value < 0.5;
    }
    require(interior, "Floating distribution must not collapse to zero");
    zeroerr::InRange<int64_t> signed_range((std::numeric_limits<int64_t>::min)(),
                                          (std::numeric_limits<int64_t>::max)());
    zeroerr::InRange<uint64_t> unsigned_range(0, UINT64_MAX);
    bool negative = false, positive = false, wide = false;
    for (int i = 0; i < 100; ++i) {
        const auto value = signed_range.GetRandomCorpus(rng);
        negative |= value < 0;
        positive |= value > 0;
        wide |= unsigned_range.GetRandomCorpus(rng) > UINT32_MAX;
    }
    require(negative && positive && wide, "Full-width integer sampling");
    zeroerr::InRange<int> ints((std::numeric_limits<int>::min)(),
                              (std::numeric_limits<int>::max)());
    int value = 0;
    ints.Mutate(rng, value, false);
    zeroerr::InRange<unsigned char> byte(0, 255);
    require(byte.GetRandomCorpus(rng) <= 255, "Small integer sampling");
    zeroerr::InRange<double> point(0.25, 0.25);
    require(point.GetRandomCorpus(rng) == 0.25, "Singleton interval");
    const double largest = (std::numeric_limits<double>::max)();
    zeroerr::InRange<double> extremes(-largest, largest);
    for (int i = 0; i < 100; ++i)
        require(std::isfinite(extremes.GetRandomCorpus(rng)), "Extreme float interval");
    rejects([] { zeroerr::InRange<int> bad(2, 1); });
    rejects([] { zeroerr::InRange<double> bad(0, std::numeric_limits<double>::infinity()); });
    rejects([] { zeroerr::InRange<double> bad(std::numeric_limits<double>::quiet_NaN(), 1); });
    const zeroerr::ElementOf<int> elements({2, 4, 6});
    for (int i = 0; i < 100; ++i) {
        int item = elements.GetRandomValue(rng);
        require(item == 2 || item == 4 || item == 6, "ElementOf instantiation and values");
    }
    rejects([] { zeroerr::ElementOf<int> empty({}); });
}

static void serialization() {
    using zeroerr::IRObject;
    using zeroerr::String;
    IRObject decoded = IRObject::FromString("\"\\x41\\x7f\\xAb\"");
    String bytes = decoded.GetScalar<String>();
    require(bytes.size() == 3 && bytes[0] == 'A' &&
                static_cast<unsigned char>(bytes[1]) == 0x7f &&
                static_cast<unsigned char>(bytes[2]) == 0xab, "Hexadecimal decoding");
    for (const char* malformed : {"\"\\x\"", "\"\\x4\"", "\"\\xGG\"", "\"\\q\"", "\"\\\"", "\"open"})
        rejects([&] { IRObject::FromString(malformed); });
    for (int n : {0, 1, 7, 8, 14, 15, 100}) {
        String original(static_cast<size_t>(n), 'z');
        IRObject source = IRObject::FromCorpus(original);
        IRObject copy(source);
        source.SetScalar(12);
        require(copy.GetScalar<String>() == original, "Independent string copy");
        IRObject moved(std::move(copy));
        require(copy.type == IRObject::Undefined, "Moved-from object");
        require(moved.GetScalar<String>() == original, "Moved string");
        moved = moved;
        moved = std::move(moved);
        require(moved.GetScalar<String>() == original, "Self assignment");
        auto parsed = IRObject::FromString(IRObject::ToString(moved));
        require(parsed.GetScalar<String>() == original, "String round trip");
    }
    // Every non-NUL byte is representable by the current C-string based IR format.
    for (unsigned c = 1; c < 256; ++c) {
        IRObject source = IRObject::FromCorpus(String(1, static_cast<char>(c)));
        auto parsed = IRObject::FromString(IRObject::ToString(source));
        require(parsed.GetScalar<String>() == source.GetScalar<String>(), "Byte round trip");
    }
    std::vector<std::vector<std::string>> values{{std::string(100, 'a')}, {"nested", "value"}};
    for (int i = 0; i < 100; ++i) {
        IRObject original = IRObject::FromCorpus(values);
        IRObject copied(original);
        original.GetChildren().children[0].SetScalar(0);
        require(IRObject::ToCorpus<decltype(values)>(copied) == values, "Deep child copy");
        IRObject assigned = IRObject::FromCorpus(std::string(100, 'b'));
        assigned = copied;
        copied.SetScalar(3.5);
        require(IRObject::ToCorpus<decltype(values)>(assigned) == values, "Deep copy assignment");
        IRObject moved = IRObject::FromCorpus(values);
        moved = std::move(assigned);
        require(assigned.type == IRObject::Undefined, "Move assignment clears source");
        auto roundtrip = IRObject::FromString(IRObject::ToString(moved));
        require(IRObject::ToCorpus<decltype(values)>(roundtrip) == values, "Nested round trip");
    }
    IRObject parent = IRObject::FromCorpus(values);
    parent = std::move(parent.GetChildren().children[0]);
    require(IRObject::ToCorpus<std::vector<std::string>>(parent) == values[0],
            "Moving an owned child into its parent");
    require(sizeof(IRObject) == 16, "IRObject layout size");
}

int main() {
    try {
        containers<std::vector<int>>();
        containers<std::set<int>>();
        ranges();
        serialization();
    } catch (const std::exception& error) {
        std::fprintf(stderr, "%s\n", error.what());
        return 1;
    }
    return 0;
}
