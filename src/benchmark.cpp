#include "zeroerr/benchmark.h"
#include "zeroerr/internal/rng.h"
#include "zeroerr/table.h"

#include <cstring>
#include <iostream>
#include <map>
#include <random>
#include <stdexcept>


// #ifdef _WIN32
// #define ZEROERR_ETW 1
// #endif


namespace zeroerr {

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
    return std::chrono::duration_cast<std::chrono::duration<double, std::nano>>(duration).count();
}

struct BenchState {
    BenchState(Benchmark& bench) : bench(bench), stage(UnInit) {
        targetEpochTime = clockResolution() * bench.minimalResolutionMutipler;
        targetEpochTime = std::max(targetEpochTime, bench.mMinEpochTime);
        targetEpochTime = std::min(targetEpochTime, bench.mMaxEpochTime);
        numEpoch        = bench.epochs;
        numIteration    = 0;
        elapsed         = Clock::duration(0);
    }

    Benchmark& bench;

    enum { UnInit, WarmUp, UpScaling, Measurement } stage;

    Clock::duration elapsed;
    Clock::duration targetEpochTime;
    uint64_t        numIteration, numEpoch;

    Rng mRng{1024};

    bool isCloseEnoughForMeasurements() const noexcept {
        return elapsed * 3 >= targetEpochTime * 2;
    }


    uint64_t calcBestNumIters() noexcept {
        double Elapsed               = d(elapsed);
        double TargetRuntimePerEpoch = d(targetEpochTime);
        double NewIters              = TargetRuntimePerEpoch * d(numIteration) / Elapsed;

        NewIters *= (1.0 + 0.2 * mRng.uniform01());

        // +1 for correct rounding when casting and make sure there are at least 1 iteration
        return static_cast<uint64_t>(NewIters + 1);
    }

    void upscale() {
        if (elapsed * 10 < targetEpochTime) {
            // we are far below the target runtime. Multiply iterations by 10 (with overflow check)
            if (numIteration * 10 < numIteration) {
                // overflow :-(
                printf("iterations overflow. Maybe your code got optimized away?\n");
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

    BenchResult result;
};
BenchState* createBenchState(Benchmark& benchmark) { return new BenchState(benchmark); }
void        destroyBenchState(BenchState* state) { delete state; }

size_t getNumIter(BenchState* state) {
    state->nextStage();
    return state->numIteration;
}

void runIteration(BenchState* state) {
    auto& pc       = PerformanceCounter::inst();
    state->elapsed = pc.elapsed;
    pc.updateResults(state->numIteration);

    if (state->stage == BenchState::Measurement) {
        PerfCountSet<double> pcset;
        pcset.iterations    = d(state->numIteration);
        pcset.timeElapsed() = d(state->elapsed) / pcset.iterations;

        for (int i = 1; i < 7; ++i) {
            if (pc.has().data[i]) {
                pcset.data[i] = d(pc.val().data[i]) / pcset.iterations;
            }
        }

        state->result.epoch_details.push_back(pcset);
    }
}

void moveResult(BenchState* state, std::string name) {
    auto& pc           = PerformanceCounter::inst();
    state->result.name = name;
    state->result.has  = pc.has();

    state->bench.result.push_back(state->result);
    destroyBenchState(state);
}


PerfCountSet<double> BenchResult::average() const {
    PerfCountSet<double> avg;
    for (int i = 0; i < 7; ++i) {
        if (has.data[i]) {
            double sum = 0;
            for (auto& pcset : epoch_details) {
                sum += pcset.data[i];
            }
            sum /= epoch_details.size();
            avg.data[i] = sum;
        }
    }
    return avg;
}
PerfCountSet<double> BenchResult::min() const {
    PerfCountSet<double> min;
    for (int i = 0; i < 7; ++i) {
        if (has.data[i]) {
            double min_val = std::numeric_limits<double>::max();
            for (auto& pcset : epoch_details) {
                min_val = std::min(min_val, pcset.data[i]);
            }
            min.data[i] = min_val;
        }
    }
    return min;
}
PerfCountSet<double> BenchResult::max() const {
    PerfCountSet<double> max;
    for (int i = 0; i < 7; ++i) {
        if (has.data[i]) {
            double max_val = std::numeric_limits<double>::min();
            for (auto& pcset : epoch_details) {
                max_val = std::max(max_val, pcset.data[i]);
            }
            max.data[i] = max_val;
        }
    }
    return max;
}
PerfCountSet<double> BenchResult::mean() const {
    PerfCountSet<double> mean;

    return mean;
}

void Benchmark::report() {
    static const char* names[] = {
        "elapsed(ns)", "page faults", "cpu_cycles", "ctx switch", "inst", "branch", "branch misses",
    };
    std::cerr << "" << title << ":" << std::endl;

    std::vector<std::string> headers{""};
    for (unsigned i = 0; i < sizeof(names) / sizeof(names[0]); i++) {
        if (result[0].has.data[i]) headers.push_back(names[i]);
    }
    Table output;
    output.set_header(headers);
    for (auto& row : result) {
        auto                row_avg = row.average();
        std::vector<double> values;
        for (int j = 0; j < 7; ++j)
            if (row.has.data[j]) values.push_back(row_avg.data[j]);
        output.add_row(row.name, values);
    }
    std::cerr << output.str() << std::endl;
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

}  // namespace detail

#ifdef ZEROERR_PERF
#include <linux/perf_event.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <unistd.h>

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
            v = std::numeric_limits<uint64_t>::max();
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

    void updateResults(uint64_t numIters) {
        // clear old data
        for (auto& id_value : mIdToTarget) {
            *id_value.second.targetValue = UINT64_C(0);
        }

        if (mHasError) return;

        mTimeEnabledNanos = mCounters[1] - mCalibratedOverhead[1];
        mTimeRunningNanos = mCounters[2] - mCalibratedOverhead[2];

        for (uint64_t i = 0; i < mCounters[0]; ++i) {
            auto idx = static_cast<size_t>(3 + i * 2 + 0);
            auto id  = mCounters[idx + 1U];

            auto it = mIdToTarget.find(id);
            if (it != mIdToTarget.end()) {
                auto& tgt        = it->second;
                *tgt.targetValue = mCounters[idx];
                if (tgt.correctMeasuringOverhead) {
                    if (*tgt.targetValue >= mCalibratedOverhead[idx]) {
                        *tgt.targetValue -= mCalibratedOverhead[idx];
                    } else {
                        *tgt.targetValue = 0U;
                    }
                }
                if (tgt.correctLoopOverhead) {
                    auto correctionVal = mLoopOverhead[idx] * numIters;
                    if (*tgt.targetValue >= correctionVal) {
                        *tgt.targetValue -= correctionVal;
                    } else {
                        *tgt.targetValue = 0U;
                    }
                }
            }
        }
    }
};
}  // namespace detail
#endif


#ifdef ZEROERR_ETW
#define INITGUID
#define NOMINMAX
#include <Windows.h>
#include <evntrace.h>
#include <wmistr.h>

namespace detail {
struct WindowsPerformanceCounter {
    TRACEHANDLE             mTraceHandle;
    std::string             name = "ZeroErr ETW";
    PEVENT_TRACE_PROPERTIES traceProperties;

    inline void beginMeasure() {
        size_t buffersize = sizeof(EVENT_TRACE_PROPERTIES) + sizeof(KERNEL_LOGGER_NAME);

        traceProperties = (PEVENT_TRACE_PROPERTIES)malloc(buffersize);
        ZeroMemory(traceProperties, buffersize);

        traceProperties->Wnode.BufferSize    = buffersize;
        traceProperties->Wnode.Flags         = WNODE_FLAG_TRACED_GUID;
        traceProperties->Wnode.Guid          = SystemTraceControlGuid;
        traceProperties->Wnode.ClientContext = 1;  // QPC clock resolution

        traceProperties->BufferSize     = 32;  // 32 KB
        traceProperties->MinimumBuffers = 32;  // 32 buffers
        traceProperties->MaximumBuffers = 32;  // 32 buffers

        traceProperties->LogFileMode = EVENT_TRACE_BUFFERING_MODE;
        traceProperties->EnableFlags = EVENT_TRACE_FLAG_CSWITCH;

        traceProperties->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);

        ULONG status = StartTrace(&mTraceHandle, KERNEL_LOGGER_NAME, traceProperties);

        if (ERROR_SUCCESS != status) {
            if (ERROR_ALREADY_EXISTS == status) {
                printf("The NT Kernel Logger session is already in use.\n");
            } else {
                printf("EnableTrace() failed with %lu\n", status);
            }
            goto cleanup;
        }

        // I got those values from here:
        // https://www.geoffchappell.com/studies/windows/km/ntoskrnl/api/ke/profobj/kprofile_source.htm
        // TotalIssues TotalCycles CacheMisses BranchMispredictions
        unsigned long perf_counter[4] = {0x02, 0x13, 0x0A, 0x0B};
        TraceSetInformation(mTraceHandle, TracePmcCounterListInfo, perf_counter,
                            sizeof(perf_counter));

    cleanup:
        if (mTraceHandle) {
            status = ControlTrace(mTraceHandle, KERNEL_LOGGER_NAME, traceProperties,
                                  EVENT_TRACE_CONTROL_STOP);

            if (ERROR_SUCCESS != status) printf("ControlTrace(stop) failed with %lu\n", status);
        }
        if (traceProperties) {
            free(traceProperties);
            traceProperties = nullptr;
        }
    }

    inline void endMeasure() {
        StopTrace(mTraceHandle, KERNEL_LOGGER_NAME, traceProperties);
        if (traceProperties) {
            free(traceProperties);
            traceProperties = nullptr;
        }
    }
};
}  // namespace detail
#endif


PerformanceCounter::PerformanceCounter() {
    _has.timeElapsed() = true;  // this should be always available
#ifdef ZEROERR_PERF
    _perf        = new detail::LinuxPerformanceCounter();
    using Target = detail::LinuxPerformanceCounter::Target;

    // clang-format off
    _has.pageFaults()         = _perf->monitor(PERF_COUNT_SW_PAGE_FAULTS,         Target{&_val.pageFaults(),         true, false});
    _has.cpuCycles()          = _perf->monitor(PERF_COUNT_HW_CPU_CYCLES,          Target{&_val.cpuCycles(),          true, false});
    _has.contextSwitches()    = _perf->monitor(PERF_COUNT_SW_CONTEXT_SWITCHES,    Target{&_val.contextSwitches(),    true, false});
    _has.instructions()       = _perf->monitor(PERF_COUNT_HW_INSTRUCTIONS,        Target{&_val.instructions(),       true, true });
    _has.branchInstructions() = _perf->monitor(PERF_COUNT_HW_BRANCH_INSTRUCTIONS, Target{&_val.branchInstructions(), true, false});
    _has.branchMisses()       = _perf->monitor(PERF_COUNT_HW_BRANCH_MISSES,       Target{&_val.branchMisses(),       true, false});
    // clang-format on

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

#ifdef ZEROERR_ETW
    win_perf = new detail::WindowsPerformanceCounter();

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
void PerformanceCounter::updateResults(uint64_t numIters) {
#ifdef ZEROERR_PERF
    _perf->updateResults(numIters);
#else
    (void)numIters;
#endif
}


}  // namespace zeroerr
