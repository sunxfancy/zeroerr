#define ZEROERR_ENABLE_PFR
#include "zeroerr/assert.h"
#include "zeroerr/dbg.h"
#include "zeroerr/print.h"
#include "zeroerr/unittest.h"
#include "zeroerr/cuda.h"

#include <assert.h>
#include <cuda_runtime.h>

#define gpuErrchk(ans) \
    { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, const char *file, int line, bool abort = true) {
    if (code != cudaSuccess) {
        fprintf(stderr, "GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
        if (abort) exit(code);
    }
}

#define N 1024
__global__ void add(int *a, int *b, int *c, unsigned n) {
    unsigned id = blockIdx.x * blockDim.x + threadIdx.x;
    if (id < n) {
        c[id] = a[id] + b[id];
    }
}

void random_ints(int *a, int k) {
    for (int i = 0; i < k; ++i) a[i] = rand();
}


TEST_CASE("CUDA basic test") {
    int *a, *b, *c;        // host copies of a, b, c
    int *d_a, *d_b, *d_c;  // device copies of a, b, c
    int  size = N * sizeof(int);
    // Allocate space for device copies of a, b, c
    gpuErrchk(cudaMalloc((void **)&d_a, size));
    gpuErrchk(cudaMalloc((void **)&d_b, size));
    gpuErrchk(cudaMalloc((void **)&d_c, size));
    // Setup input values
    a = (int *)malloc(size);
    random_ints(a, N);
    b = (int *)malloc(size);
    random_ints(b, N);
    c = (int *)malloc(size);

    // Copy inputs to device
    gpuErrchk(cudaMemcpy(d_a, a, size, cudaMemcpyHostToDevice));
    gpuErrchk(cudaMemcpy(d_b, b, size, cudaMemcpyHostToDevice));
    // Launch add() kernel on GPU
    add<<<N / 32, 32>>>(d_a, d_b, d_c, N);
    gpuErrchk(cudaGetLastError());
    gpuErrchk(cudaDeviceSynchronize());
    // Copy result back to host
    gpuErrchk(cudaMemcpy(c, d_c, size, cudaMemcpyDeviceToHost));

    dbg(c);
    for (int i = 0; i < N; ++i) {
        CHECK(c[i] == (a[i] + b[i]));
    }

    // Cleanup
    free(a);
    free(b);
    free(c);
    gpuErrchk(cudaFree(d_a));
    gpuErrchk(cudaFree(d_b));
    gpuErrchk(cudaFree(d_c));
}