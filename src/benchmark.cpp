#include "zeroerr/benchmark.h"

#include <cstring>
#include <iostream>
#include <random>
#include <stdexcept>

#ifdef ZEROERR_PERF
#include <linux/perf_event.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <map>
#endif

namespace zeroerr {

#ifdef ZEROERR_PERF
namespace detail {
struct LinuxPerformanceCounter {
    inline void beginMeasure() {
        if (mHasError) return;

        mHasError = -1 == ioctl(mFd, PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
        if (mHasError) return;

        mHasError = -1 == ioctl(mFd, PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);
    }
    inline void endMeasure() {
        if (mHasError) return;

        mHasError = (-1 == ioctl(mFd, PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP));
        if (mHasError) return;

        auto const numBytes = sizeof(uint64_t) * mCounters.size();
        auto       ret      = read(mFd, mCounters.data(), numBytes);
        mHasError           = ret != static_cast<ssize_t>(numBytes);
    }


    // rounded integer division
    template <typename T>
    static inline T divRounded(T a, T divisor) {
        return (a + divisor / 2) / divisor;
    }

    static inline uint32_t mix(uint32_t x) noexcept {
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        return x;
    }

    template <typename Op>
    void calibrate(Op&& op) {
        // clear current calibration data,
        for (auto& v : mCalibratedOverhead) {
            v = UINT64_C(0);
        }

        // create new calibration data
        auto newCalibration = mCalibratedOverhead;
        for (auto& v : newCalibration) {
            v = (std::numeric_limits<uint64_t>::max)();
        }
        for (size_t iter = 0; iter < 100; ++iter) {
            beginMeasure();
            op();
            endMeasure();
            if (mHasError) return;

            for (size_t i = 0; i < newCalibration.size(); ++i) {
                auto diff = mCounters[i];
                if (newCalibration[i] > diff) {
                    newCalibration[i] = diff;
                }
            }
        }

        mCalibratedOverhead = std::move(newCalibration);

        {
            // calibrate loop overhead. For branches & instructions this makes sense, not so much
            // for everything else like cycles. marsaglia's xorshift: mov, sal/shr, xor. Times 3.
            // This has the nice property that the compiler doesn't seem to be able to optimize
            // multiple calls any further. see https://godbolt.org/z/49RVQ5
            uint64_t const numIters = 100000U + (std::random_device{}() & 3);
            uint64_t       n        = numIters;
            uint32_t       x        = 1234567;

            beginMeasure();
            while (n-- > 0) {
                x = mix(x);
            }
            endMeasure();
            detail::doNotOptimizeAway(x);
            auto measure1 = mCounters;

            n = numIters;
            beginMeasure();
            while (n-- > 0) {
                // we now run *twice* so we can easily calculate the overhead
                x = mix(x);
                x = mix(x);
            }
            endMeasure();
            detail::doNotOptimizeAway(x);
            auto measure2 = mCounters;

            for (size_t i = 0; i < mCounters.size(); ++i) {
                // factor 2 because we have two instructions per loop
                auto m1 =
                    measure1[i] > mCalibratedOverhead[i] ? measure1[i] - mCalibratedOverhead[i] : 0;
                auto m2 =
                    measure2[i] > mCalibratedOverhead[i] ? measure2[i] - mCalibratedOverhead[i] : 0;
                auto overhead = m1 * 2 > m2 ? m1 * 2 - m2 : 0;

                mLoopOverhead[i] = divRounded(overhead, numIters);
            }
        }
    }


    struct Target {
        uint64_t* targetValue;
        bool      correctMeasuringOverhead;
        bool      correctLoopOverhead;
    };

    std::map<uint64_t, Target> mIdToTarget{};

    // start with minimum size of 3 for read_format
    std::vector<uint64_t> mCounters{3};
    std::vector<uint64_t> mCalibratedOverhead{3};
    std::vector<uint64_t> mLoopOverhead{3};

    uint64_t mTimeEnabledNanos = 0;
    uint64_t mTimeRunningNanos = 0;

    int  mFd       = -1;
    bool mHasError = false;

    ~LinuxPerformanceCounter() {
        if (mFd != -1) close(mFd);
    }

    bool monitor(perf_sw_ids swId, Target target) {
        return monitor(PERF_TYPE_SOFTWARE, swId, target);
    }

    bool monitor(perf_hw_id hwId, Target target) {
        return monitor(PERF_TYPE_HARDWARE, hwId, target);
    }

    bool monitor(uint32_t type, uint64_t eventid, Target target) {
        *target.targetValue = (std::numeric_limits<uint64_t>::max)();
        if (mHasError) return false;

        auto pea = perf_event_attr();
        std::memset(&pea, 0, sizeof(perf_event_attr));
        pea.type           = type;
        pea.size           = sizeof(perf_event_attr);
        pea.config         = eventid;
        pea.disabled       = 1;  // start counter as disabled
        pea.exclude_kernel = 1;
        pea.exclude_hv     = 1;

        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        pea.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID | PERF_FORMAT_TOTAL_TIME_ENABLED |
                          PERF_FORMAT_TOTAL_TIME_RUNNING;

        const int pid = 0;         // the current process
        const int cpu = -1;        // all CPUs
#if defined(PERF_FLAG_FD_CLOEXEC)  // since Linux 3.14
        const unsigned long flags = PERF_FLAG_FD_CLOEXEC;
#else
        const unsigned long flags = 0;
#endif

        auto fd = static_cast<int>(syscall(__NR_perf_event_open, &pea, pid, cpu, mFd, flags));
        if (-1 == fd) return false;
        // first call: set to fd, and use this from now on
        if (-1 == mFd) mFd = fd;

        uint64_t id = 0;
        if (-1 == ioctl(fd, PERF_EVENT_IOC_ID, &id)) return false;

        // insert into map, rely on the fact that map's references are constant.
        mIdToTarget.emplace(id, target);

        // prepare readformat with the correct size (after the insert)
        auto size = 3 + 2 * mIdToTarget.size();
        mCounters.resize(size);
        mCalibratedOverhead.resize(size);
        mLoopOverhead.resize(size);

        return true;
    }
};
}  // namespace detail
#endif

#pragma region PerformanceCounter

PerformanceCounter::PerformanceCounter() {
#ifdef ZEROERR_PERF
    _perf        = new detail::LinuxPerformanceCounter();
    using Target = detail::LinuxPerformanceCounter::Target;
    _has.pageFaults =
        _perf->monitor(PERF_COUNT_SW_PAGE_FAULTS, Target{&_val.pageFaults, true, false});
    _has.cpuCycles =
        _perf->monitor(PERF_COUNT_HW_REF_CPU_CYCLES, Target{&_val.cpuCycles, true, false});
    _has.contextSwitches =
        _perf->monitor(PERF_COUNT_SW_CONTEXT_SWITCHES, Target{&_val.contextSwitches, true, false});
    _has.instructions =
        _perf->monitor(PERF_COUNT_HW_INSTRUCTIONS, Target{&_val.instructions, true, true});
    _has.branchInstructions = _perf->monitor(PERF_COUNT_HW_BRANCH_INSTRUCTIONS,
                                             Target{&_val.branchInstructions, true, false});
    _has.branchMisses =
        _perf->monitor(PERF_COUNT_HW_BRANCH_MISSES, Target{&_val.branchMisses, true, false});
    // _has.branchMisses = false;

    _perf->calibrate([] {
        auto before = Clock::now();
        auto after  = Clock::now();
        (void)before;
        (void)after;
    });

    if (_perf->mHasError) {
        // something failed, don't monitor anything.
        _has = PerfCountSet<bool>{};
    }
#endif
}
PerformanceCounter::~PerformanceCounter() {
#ifdef ZEROERR_PERF
    delete _perf;
#endif
}

PerformanceCounter& PerformanceCounter::inst() {
    static PerformanceCounter counter;
    return counter;
}

void PerformanceCounter::beginMeasure() {
#ifdef ZEROERR_PERF
    _perf->beginMeasure();
#endif
    _start = Clock::now();
}
void PerformanceCounter::endMeasure() {
    elapsed = Clock::now() - _start;
#ifdef ZEROERR_PERF
    _perf->endMeasure();
#endif
}
void PerformanceCounter::updateResults(uint64_t numIters) {}

#pragma endregion

// determines resolution of the given clock. This is done by measuring multiple times and returning
// the minimum time difference.
Clock::duration calcClockResolution(size_t numEvaluations) noexcept {
    auto              bestDuration = Clock::duration::max();
    Clock::time_point tBegin;
    Clock::time_point tEnd;
    for (size_t i = 0; i < numEvaluations; ++i) {
        tBegin = Clock::now();
        do {
            tEnd = Clock::now();
        } while (tBegin == tEnd);
        bestDuration = (std::min)(bestDuration, tEnd - tBegin);
    }
    return bestDuration;
}

// Calculates clock resolution once, and remembers the result
Clock::duration clockResolution() noexcept {
    static Clock::duration sResolution = calcClockResolution(20);
    return sResolution;
}

// helpers to get double values
template <typename T>
static inline double d(T t) noexcept {
    return static_cast<double>(t);
}
static inline double d(Clock::duration duration) noexcept {
    return std::chrono::duration_cast<std::chrono::duration<double>>(duration).count();
}

#pragma region BenchState
struct BenchState {
    BenchState(Benchmark& bench) : bench(bench), stage(UnInit) {
        targetEpochTime = clockResolution() * bench.minimalResolutionMutipler;
        targetEpochTime = std::max(targetEpochTime, bench.mMinEpochTime);
        targetEpochTime = std::min(targetEpochTime, bench.mMaxEpochTime);
        numEpoch        = bench.epochs;
    }

    Benchmark& bench;

    enum { UnInit, WarmUp, UpScaling, Measurement } stage;

    Clock::duration elapsed;
    Clock::duration targetEpochTime;
    uint64_t        numIteration, numEpoch;

    detail::Rng mRng{1024};

    bool isCloseEnoughForMeasurements() const noexcept {
        return elapsed * 3 >= targetEpochTime * 2;
    }


    uint64_t calcBestNumIters() noexcept {
        double Elapsed               = d(elapsed);
        double TargetRuntimePerEpoch = d(targetEpochTime);
        double NewIters              = TargetRuntimePerEpoch / Elapsed * d(numIteration);

        NewIters *= 1.0 + 0.2 * mRng.uniform01();

        // +0.5 for correct rounding when casting
        return static_cast<uint64_t>(NewIters + 0.5);
    }

    void upscale() {
        if (elapsed * 10 < targetEpochTime) {
            // we are far below the target runtime. Multiply iterations by 10 (with overflow check)
            if (numIteration * 10 < numIteration) {
                // overflow :-(
                printf("iterations overflow. Maybe your code got optimized away?");
                numIteration = 0;
                return;
            }
            if (elapsed * 100 < targetEpochTime)
                numIteration *= 100;
            else
                numIteration *= 10;
        } else {
            numIteration = calcBestNumIters();
        }
    }

    void nextStage() noexcept {
        switch (stage) {
            case UnInit:
                if (bench.warmup != 0) {
                    stage        = WarmUp;
                    numIteration = bench.warmup;
                } else if (bench.iter_per_epoch != 0) {
                    stage        = Measurement;
                    numIteration = bench.iter_per_epoch;
                } else {
                    stage        = UpScaling;
                    numIteration = 1;
                    nextStage();
                }
                break;
            case WarmUp:
                if (bench.iter_per_epoch != 0) {
                    stage        = Measurement;
                    numIteration = bench.iter_per_epoch;
                } else if (isCloseEnoughForMeasurements()) {
                    stage        = Measurement;
                    numIteration = calcBestNumIters();
                } else {
                    stage = UpScaling;
                    nextStage();
                }
                break;
            case UpScaling:
                if (isCloseEnoughForMeasurements()) {
                    stage        = Measurement;
                    numIteration = calcBestNumIters();
                } else {
                    stage = UpScaling;
                    upscale();
                }
                break;
            case Measurement:
                if (numEpoch) {
                    numEpoch--;
                } else {
                    numIteration = 0;
                }
                break;
        }
    }
};
BenchState* createBenchState(Benchmark& benchmark) { return new BenchState(benchmark); }
void        destroyBenchState(BenchState* state) { delete state; }

size_t getNumIter(BenchState* state) {
    state->nextStage();
    printf("%lu  %s %lu\n", state->numIteration,
           state->stage == BenchState::WarmUp      ? "warmup"
           : state->stage == BenchState::UpScaling ? "upscale"
                                                   : "measurement",
           state->numEpoch);
    return state->numIteration;
}

void runIteration(BenchState* state) { state->elapsed = PerformanceCounter::inst().elapsed; }

void moveResult(BenchState* state) {
    auto& pc = PerformanceCounter::inst();

    PerfCountSet<double> pcset;
    state->bench.result.results.push_back(pcset);
    destroyBenchState(state);
}

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
