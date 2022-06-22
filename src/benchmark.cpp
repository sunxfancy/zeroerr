#include "zeroerr/benchmark.h"

namespace zeroerr {


PerformanceCounter& PerformanceCounter::inst() {
    static PerformanceCounter counter;
    return counter;
}


struct BenchState {};
BenchState* createBenchState(Benchmark& benchmark) { return new BenchState(); }
void        destroyBenchState(BenchState* state) { delete state; }

size_t getNumIter(BenchState* state) { return 0; }
void   runIteration(BenchState* state, PerformanceCounter& counter) {}
void   moveResult(BenchState* state) {}


}  // namespace zeroerr
