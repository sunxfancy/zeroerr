/*
 * This benchmark component is modified from nanobench by Martin Ankerl
 * https://github.com/martinus/nanobench
 */

#pragma once
#include "zeroerr/internal/config.h"

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>


ZEROERR_SUPPRESS_COMMON_WARNINGS_PUSH

#define ZEROERR_CREATE_BENCHMARK_FUNC(function, name)                    \
    static void                     function(zeroerr::TestContext*);     \
    static zeroerr::detail::regTest ZEROERR_NAMEGEN(_zeroerr_reg)(       \
        {name, __FILE__, __LINE__, function}, zeroerr::TestType::bench); \
    static void function(ZEROERR_UNUSED(zeroerr::TestContext* _ZEROERR_TEST_CONTEXT))

#define BENCHMARK(name) ZEROERR_CREATE_BENCHMARK_FUNC(ZEROERR_NAMEGEN(_zeroerr_benchmark), name)


namespace zeroerr {

/**
 * @brief PerfCountSet is a set of performance counters.
 */
template <typename T>
struct PerfCountSet {
    T iterations{};
    T data[7]{};

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
struct WindowsPerformanceCounter;
}  // namespace detail

/**
 * @brief PerformanceCounter is a class to measure the performance of a function.
 */
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

    detail::LinuxPerformanceCounter*   _perf    = nullptr;
    detail::WindowsPerformanceCounter* win_perf = nullptr;
};

/**
 * @brief BenchResult is a result of running the benchmark.
 */
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
    const char* time_unit      = "ns";
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

}  // namespace zeroerr

ZEROERR_SUPPRESS_COMMON_WARNINGS_POP