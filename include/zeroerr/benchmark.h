#pragma once
#include "zeroerr/internal/config.h"

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

namespace zeroerr {


#pragma region Benchmark


template <typename T>
struct PerfCountSet {
    T iterations;
    T data[7];

    T& timeElapsed() { return data[0]; }
    T& pageFaults() { return data[1]; }
    T& cpuCycles() { return data[2]; }
    T& contextSwitches() { return data[3]; }
    T& instructions() { return data[4]; }
    T& branchInstructions() { return data[5]; }
    T& branchMisses() { return data[6]; }
};

using Clock = std::conditional<std::chrono::high_resolution_clock::is_steady,
                               std::chrono::high_resolution_clock, std::chrono::steady_clock>::type;

namespace detail {
struct LinuxPerformanceCounter;
}

struct PerformanceCounter {
    PerformanceCounter();
    ~PerformanceCounter();

    void beginMeasure();
    void endMeasure();
    void updateResults(uint64_t numIters);

    PerfCountSet<uint64_t> const& val() const noexcept { return _val; }
    PerfCountSet<bool>            has() const noexcept { return _has; }

    static PerformanceCounter& inst();

    Clock::duration elapsed;

protected:
    Clock::time_point      _start;
    PerfCountSet<uint64_t> _val;
    PerfCountSet<bool>     _has;

    detail::LinuxPerformanceCounter* _perf = nullptr;
};


struct BenchResult {
    enum Measure {
        time_elapsed        = 1 << 0,
        iterations          = 1 << 1,
        page_faults         = 1 << 2,
        cpu_cycles          = 1 << 3,
        context_switches    = 1 << 4,
        instructions        = 1 << 5,
        branch_instructions = 1 << 6,
        branch_misses       = 1 << 7,
        all                 = (1 << 8) - 1,
    };
    std::string                       name;
    std::vector<PerfCountSet<double>> epoch_details;
    PerfCountSet<bool>                has;

    PerfCountSet<double> average() const;
    PerfCountSet<double> min() const;
    PerfCountSet<double> max() const;
    PerfCountSet<double> mean() const;
};

struct Benchmark;
struct BenchState;
BenchState* createBenchState(Benchmark& benchmark);
void        destroyBenchState(BenchState* state);

size_t getNumIter(BenchState* state);
void   runIteration(BenchState* state);
void   moveResult(BenchState* state, std::string name);


/**
 * @brief Benchmark create a core object for configuration of a benchmark.
 * This class is a driver to run multiple times of a benchmark. Each time of a run will generate a
 * row of data. Report will print the data in console.
 */
struct Benchmark {
    std::string title          = "benchmark";
    const char* op_unit        = "op";
    const char* time_unit      = "ms";
    uint64_t    epochs         = 10;
    uint64_t    warmup         = 0;
    uint64_t    iter_per_epoch = 0;

    using ns   = std::chrono::nanoseconds;
    using ms   = std::chrono::milliseconds;
    using time = ns;

    time mMaxEpochTime = ms(100);
    time mMinEpochTime = ms(1);

    uint64_t minimalResolutionMutipler = 1000;

    Benchmark(std::string title) { this->title = title; }


    template <typename Op>
    Benchmark& run(std::string name, Op&& op) {
        auto* s  = createBenchState(*this);
        auto& pc = PerformanceCounter::inst();
        while (auto n = getNumIter(s)) {
            pc.beginMeasure();
            while (n-- > 0) op();
            pc.endMeasure();
            runIteration(s);
        }
        moveResult(s, name);
        return *this;
    }

    template <typename Op>
    Benchmark& run(Op&& op) {
        return run("", std::forward<Op>(op));
    }

    std::vector<BenchResult> result;
    void                     report();
};


#pragma endregion

#pragma region details


namespace detail {

#if defined(_MSC_VER)
void doNotOptimizeAwaySink(void const*);

template <typename T>
void doNotOptimizeAway(T const& val) {
    doNotOptimizeAwaySink(&val);
}

#else

// These assembly magic is directly from what Google Benchmark is doing. I have previously used
// what facebook's folly was doing, but this seemed to have compilation problems in some cases.
// Google Benchmark seemed to be the most well tested anyways. see
// https://github.com/google/benchmark/blob/master/include/benchmark/benchmark.h#L307
template <typename T>
void doNotOptimizeAway(T const& val) {
    // NOLINTNEXTLINE(hicpp-no-assembler)
    asm volatile("" : : "r,m"(val) : "memory");
}

template <typename T>
void doNotOptimizeAway(T& val) {
#if defined(__clang__)
    // NOLINTNEXTLINE(hicpp-no-assembler)
    asm volatile("" : "+r,m"(val) : : "memory");
#else
    // NOLINTNEXTLINE(hicpp-no-assembler)
    asm volatile("" : "+m,r"(val) : : "memory");
#endif
}
#endif


/**
 * An extremely fast random generator. Currently, this implements *RomuDuoJr*, developed by Mark
 * Overton. Source: http://www.romu-random.org/
 *
 * RomuDuoJr is extremely fast and provides reasonable good randomness. Not enough for large
 * jobs, but definitely good enough for a benchmarking framework.
 *
 *  * Estimated capacity: @f$ 2^{51} @f$ bytes
 *  * Register pressure: 4
 *  * State size: 128 bits
 *
 * This random generator is a drop-in replacement for the generators supplied by ``<random>``.
 * It is not cryptographically secure. It's intended purpose is to be very fast so that
 * benchmarks that make use of randomness are not distorted too much by the random generator.
 *
 * Rng also provides a few non-standard helpers, optimized for speed.
 */
class Rng final {
public:
    /**
     * @brief This RNG provides 64bit randomness.
     */
    using result_type = uint64_t;

    static constexpr uint64_t(min)();
    static constexpr uint64_t(max)();

    /**
     * As a safety precausion, we don't allow copying. Copying a PRNG would mean you would have
     * two random generators that produce the same sequence, which is generally not what one
     * wants. Instead create a new rng with the default constructor Rng(), which is
     * automatically seeded from `std::random_device`. If you really need a copy, use copy().
     */
    Rng(Rng const&) = delete;

    /**
     * Same as Rng(Rng const&), we don't allow assignment. If you need a new Rng create one with
     * the default constructor Rng().
     */
    Rng& operator=(Rng const&) = delete;

    // moving is ok
    Rng(Rng&&) noexcept            = default;
    Rng& operator=(Rng&&) noexcept = default;
    ~Rng() noexcept                = default;

    /**
     * @brief Creates a new Random generator with random seed.
     *
     * Instead of a default seed (as the random generators from the STD), this properly seeds
     * the random generator from `std::random_device`. It guarantees correct seeding. Note that
     * seeding can be relatively slow, depending on the source of randomness used. So it is best
     * to create a Rng once and use it for all your randomness purposes.
     */
    Rng();

    /*!
      Creates a new Rng that is seeded with a specific seed. Each Rng created from the same seed
      will produce the same randomness sequence. This can be useful for deterministic behavior.
      @verbatim embed:rst
      .. note::
         The random algorithm might change between nanobench releases. Whenever a faster and/or
      better random generator becomes available, I will switch the implementation.
      @endverbatim
      As per the Romu paper, this seeds the Rng with splitMix64 algorithm and performs 10
      initial rounds for further mixing up of the internal state.
      @param seed  The 64bit seed. All values are allowed, even 0.
     */
    explicit Rng(uint64_t seed) noexcept;
    Rng(uint64_t x, uint64_t y) noexcept;
    Rng(std::vector<uint64_t> const& data);

    /**
     * Creates a copy of the Rng, thus the copy provides exactly the same random sequence as the
     * original.
     */
    Rng copy() const noexcept;

    /**
     * @brief Produces a 64bit random value. This should be very fast, thus it is marked as
     * inline. In my benchmark, this is ~46 times faster than `std::default_random_engine` for
     * producing 64bit random values. It seems that the fastest std contender is
     * `std::mt19937_64`. Still, this RNG is 2-3 times as fast.
     *
     * @return uint64_t The next 64 bit random value.
     */
    inline uint64_t operator()() noexcept;

    // This is slightly biased. See

    /**
     * Generates a random number between 0 and range (excluding range).
     *
     * The algorithm only produces 32bit numbers, and is slightly biased. The effect is quite
     * small unless your range is close to the maximum value of an integer. It is possible to
     * correct the bias with rejection sampling (see
     * [here](https://lemire.me/blog/2016/06/30/fast-random-shuffling/), but this is most likely
     * irrelevant in practices for the purposes of this Rng.
     *
     * See Daniel Lemire's blog post [A fast alternative to the modulo
     * reduction](https://lemire.me/blog/2016/06/27/a-fast-alternative-to-the-modulo-reduction/)
     *
     * @param range Upper exclusive range. E.g a value of 3 will generate random numbers 0,
     * 1, 2.
     * @return uint32_t Generated random values in range [0, range(.
     */
    inline uint32_t bounded(uint32_t range) noexcept;

    // random double in range [0, 1(
    // see http://prng.di.unimi.it/

    /**
     * Provides a random uniform double value between 0 and 1. This uses the method described in
     * [Generating uniform doubles in the unit interval](http://prng.di.unimi.it/), and is
     * extremely fast.
     *
     * @return double Uniformly distributed double value in range [0,1(, excluding 1.
     */
    inline double uniform01() noexcept;

    /**
     * Shuffles all entries in the given container. Although this has a slight bias due to the
     * implementation of bounded(), this is preferable to `std::shuffle` because it is over 5
     * times faster. See Daniel Lemire's blog post [Fast random
     * shuffling](https://lemire.me/blog/2016/06/30/fast-random-shuffling/).
     *
     * @param container The whole container will be shuffled.
     */
    template <typename Container>
    void shuffle(Container& container) noexcept;

    /**
     * Extracts the full state of the generator, e.g. for serialization. For this RNG this is
     * just 2 values, but to stay API compatible with future implementations that potentially
     * use more state, we use a vector.
     *
     * @return Vector containing the full state:
     */
    std::vector<uint64_t> state() const;

private:
    static constexpr uint64_t rotl(uint64_t x, unsigned k) noexcept;

    uint64_t mX;
    uint64_t mY;
};

}  // namespace detail


/**
 * @brief Makes sure none of the given arguments are optimized away by the compiler.
 *
 * @tparam Arg Type of the argument that shouldn't be optimized away.
 * @param arg The input that we mark as being used, even though we don't do anything with it.
 */
template <typename Arg>
void doNotOptimizeAway(Arg&& arg) {
    detail::doNotOptimizeAway(std::forward<Arg>(arg));
}


#pragma endregion

}  // namespace zeroerr
