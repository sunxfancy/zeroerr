#include "zeroerr/fuzztest.h"
#include "zeroerr/log.h"

#include <cstring>

extern "C" int LLVMFuzzerRunDriver(int* argc, char*** argv,
                                   int (*user_callback)(const uint8_t* data,
                                                        size_t size));

extern "C" size_t LLVMFuzzerCustomMutator(uint8_t* data, size_t size,
                                          size_t max_size, unsigned int seed);

extern "C" std::string MutateData(const uint8_t* data, size_t size, size_t max_size, unsigned int seed);

size_t LLVMFuzzerCustomMutator(uint8_t* data, size_t size, size_t max_size,
                               unsigned int seed) {
  
  const std::string mutated_data = MutateData(data, size, max_size, seed);
  if (mutated_data.size() > max_size) {
    WARN("Mutated data is larger than the limit. Returning the original data");
    return size;
  }
  memcpy(data, mutated_data.data(), mutated_data.size());
  return mutated_data.size();
}

std::string MutateData(const uint8_t* data, size_t size, size_t max_size, unsigned int seed) {
  std::string mutated_data = std::string((const char*)data);
  return mutated_data;
}

void RunFuzzTest(int seed = 0, int runs = 1000, int max_len = 0, int timeout = 1200, int len_control = 100) {
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

    LLVMFuzzerRunDriver(&argc, &argv_c, [](const uint8_t* data, size_t size) -> int {
        return 0;
    });

}