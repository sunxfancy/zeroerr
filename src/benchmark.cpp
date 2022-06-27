#include "zeroerr/benchmark.h"

#include <cstring>
#include <iostream>
#include <random>
#include <stdexcept>
namespace zeroerr {

#ifdef ZEROERR_PERF
namespace detail {
struct LinuxPerformanceCounter;
}
#endif

#pragma region PerformanceCounter

PerformanceCounter& PerformanceCounter::inst() {
    static PerformanceCounter counter;
#ifdef ZEROERR_PERF
    counter._perf = new detail::LinuxPerformanceCounter();
#endif
    return counter;
}

void PerformanceCounter::beginMeasure() {
#ifdef ZEROERR_PERF
    _perf->beginMeasure();
#endif
    _start = Clock::now();
}
void PerformanceCounter::endMeasure() {
    Clock::time_point _end = Clock::now();

#ifdef ZEROERR_PERF
    _perf->endMeasure();
#endif

    Clock::duration elapsed = _end - _start;
    _val.timeElapsed += elapsed.count();
}
void PerformanceCounter::updateResults(uint64_t numIters) {}

#pragma endregion


#pragma region BenchState
struct BenchState {};
BenchState* createBenchState(Benchmark& benchmark) { return new BenchState(); }
void        destroyBenchState(BenchState* state) { delete state; }

size_t getNumIter(BenchState* state) { return 0; }
void   runIteration(BenchState* state, PerformanceCounter& counter) {}
void   moveResult(BenchState* state) { destroyBenchState(state); }

#pragma endregion

void Benchmark::report() {
    std::cout << "======< " << title << " >======" << std::endl;
    int i = 0;
    for (auto& row : result.results) {
        std::cerr << result.names[i] << " | " << row.timeElapsed << std::endl;
        ++i;
    }
}


namespace detail {
// Windows version of doNotOptimizeAway
// see https://github.com/google/benchmark/blob/master/include/benchmark/benchmark.h#L307
// see https://github.com/facebook/folly/blob/master/folly/Benchmark.h#L280
// see https://docs.microsoft.com/en-us/cpp/preprocessor/optimize
#if defined(_MSC_VER)
#pragma optimize("", off)
void doNotOptimizeAwaySink(void const*) {}
#pragma optimize("", on)
#endif

#ifdef ZEROERR_PERF
struct LinuxPerformanceCounter {
    void beginMeasure() { _start = Clock::now(); }
    void endMeasure() { _end = Clock::now(); }
};
};
#endif

#pragma region Rng random number generator

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


constexpr uint64_t(Rng::min)() { return 0; }

constexpr uint64_t(Rng::max)() { return (std::numeric_limits<uint64_t>::max)(); }

uint64_t Rng::operator()() noexcept {
    auto x = mX;

    mX = UINT64_C(15241094284759029579) * mY;
    mY = rotl(mY - x, 27);

    return x;
}

uint32_t Rng::bounded(uint32_t range) noexcept {
    uint64_t r32         = static_cast<uint32_t>(operator()());
    auto     multiresult = r32 * range;
    return static_cast<uint32_t>(multiresult >> 32U);
}

double Rng::uniform01() noexcept {
    auto i = (UINT64_C(0x3ff) << 52U) | (operator()() >> 12U);
    // can't use union in c++ here for type puning, it's undefined behavior.
    // std::memcpy is optimized anyways.
    double d;
    std::memcpy(&d, &i, sizeof(double));
    return d - 1.0;
}

template <typename Container>
void Rng::shuffle(Container& container) noexcept {
    auto size = static_cast<uint32_t>(container.size());
    for (auto i = size; i > 1U; --i) {
        using std::swap;
        auto p = bounded(i);  // number in [0, i)
        swap(container[i - 1], container[p]);
    }
}

constexpr uint64_t Rng::rotl(uint64_t x, unsigned k) noexcept {
    return (x << k) | (x >> (64U - k));
}

#pragma endregion

}  // namespace detail


}  // namespace zeroerr
