#include "zeroerr/fuzztest.h"
#include "zeroerr/log.h"
#include "zeroerr/assert.h"

#include <cstring>

extern "C" int LLVMFuzzerRunDriver(int* argc, char*** argv,
                                   int (*user_callback)(const uint8_t* data,
                                                        size_t size));

extern "C" size_t LLVMFuzzerCustomMutator(uint8_t* data, size_t size,
                                          size_t max_size, unsigned int seed);

namespace zeroerr {
static IFuzzTest* current_fuzz_test = nullptr;
}  // namespace zeroerr

size_t LLVMFuzzerCustomMutator(uint8_t* data, size_t size, size_t max_size,
                               unsigned int seed) {
  const std::string mutated_data = zeroerr::current_fuzz_test->MutateData(data, size, max_size, seed);
  if (mutated_data.size() > max_size) {
    WARN("Mutated data is larger than the limit. Returning the original data");
    return size;
  }
  memcpy(data, mutated_data.data(), mutated_data.size());
  return mutated_data.size();
}

namespace zeroerr {

void RunFuzzTest(IFuzzTest& fuzz_test, int seed, int runs, int max_len, int timeout, int len_control) {
#ifdef ZEROERR_ENABLE_FUZZING
    current_fuzz_test = &fuzz_test;
    int argc = 6;
    std::string argv[] = {
        "fuzztest",
        "-max_len=" + std::to_string(max_len),
        "-timeout=" + std::to_string(timeout),
        "-runs=" + std::to_string(runs),
        "-seed=" + std::to_string(seed),
        "-len_control=" + std::to_string(len_control),
    };
    char** argv_c = new char*[argc];
    for (int i = 0; i < argc; i++) {
        argv_c[i] = (char*)argv[i].c_str();
    }
    LOG("Running fuzz test");
    
    LLVMFuzzerRunDriver(&argc, &argv_c, [](const uint8_t* data, size_t size) -> int {
        LOG("Running RunOneTime");
        current_fuzz_test->RunOneTime(data, size);
        return 0;
    });

    current_fuzz_test = nullptr;
#endif
}

}  // namespace zeroerr