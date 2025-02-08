#define ZEROERR_IMPLEMENTATION
#define ZEROERR_FUZZING
#include "zeroerr.hpp"

using namespace zeroerr;


    
int find_the_biggest(std::vector<int> vec) {
    if (vec.empty()) {
        WARN("Empty vector, vec.size() = {size}", vec.size());
        return 0;
    } 
    int max = 0;
    for (int i = 0; i < vec.size(); ++i) {
        if (vec[i] > max) {max = vec[i];} 
    }
    return max;
}

FUZZ_TEST_CASE("presentation") {
    LOG("Run fuzz_test");
    FUZZ_FUNC([=](std::vector<int> vec) {
        int ans = find_the_biggest(vec);
        // verify the result
        for (int i = 0; i < vec.size(); ++i) {
            CHECK(ans >= vec[i]);
        }
        if (vec.size() == 0) {
            CHECK(ans == 0);
            // verify WARN message to make sure the path is correct
            CHECK(LOG_GET(find_the_biggest, 
            "Empty vector, vec.size() = {size}", size, size_t) == 0);
        }
    })
        .WithDomains(ContainerOf<std::vector<int>>(
            InRange<int>(0, 100)))
        .WithSeeds({{{0, 1, 2, 3, 4, 5}}, {{1, 8, 4, 2, 3}}})
        .Run(100);

}