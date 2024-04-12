#include "zeroerr/internal/rng.h"

#include <random>
#include <stdexcept>
#include <string>

namespace zeroerr {

Rng::Rng() : mX(0), mY(0) {
    std::random_device                      rd;
    std::uniform_int_distribution<uint64_t> dist;
    do {
        mX = dist(rd);
        mY = dist(rd);
    } while (mX == 0 && mY == 0);
}

static uint64_t splitMix64(uint64_t& state) noexcept {
    uint64_t z = (state += UINT64_C(0x9e3779b97f4a7c15));
    z          = (z ^ (z >> 30U)) * UINT64_C(0xbf58476d1ce4e5b9);
    z          = (z ^ (z >> 27U)) * UINT64_C(0x94d049bb133111eb);
    return z ^ (z >> 31U);
}

// Seeded as described in romu paper (update april 2020)
Rng::Rng(uint64_t seed) noexcept : mX(splitMix64(seed)), mY(splitMix64(seed)) {
    for (size_t i = 0; i < 10; ++i) {
        operator()();
    }
}

// only internally used to copy the RNG.
Rng::Rng(uint64_t x, uint64_t y) noexcept : mX(x), mY(y) {}

Rng Rng::copy() const noexcept { return Rng{mX, mY}; }

Rng::Rng(std::vector<uint64_t> const& data) : mX(0), mY(0) {
    if (data.size() != 2) {
        throw std::runtime_error("Rng::Rng: needed exactly 2 entries in data, but got " +
                                 std::to_string(data.size()));
    }
    mX = data[0];
    mY = data[1];
}

std::vector<uint64_t> Rng::state() const {
    std::vector<uint64_t> data(2);
    data[0] = mX;
    data[1] = mY;
    return data;
}


uint64_t Rng::min() { return 0; }

uint64_t Rng::max() { return (std::numeric_limits<uint64_t>::max)(); }


uint64_t Rng::rotl(uint64_t x, unsigned k) noexcept { return (x << k) | (x >> (64U - k)); }


}  // namespace zeroerr
