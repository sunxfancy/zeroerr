// Keep this workload identical when comparing revisions. Distinct operand types
// exercise the format and assertion templates without the test runner overhead.
#include "zeroerr/format.h"
#include "zeroerr/internal/decomposition.h"
#include <iostream>
#if __has_include("zeroerr/std_adapter.h")
#include "zeroerr/std_adapter.h"
#endif

template <int N> struct Value { int value; };
template <int N> std::ostream& operator<<(std::ostream& os, Value<N> v) {
    return os << v.value;
}
template <int N> bool operator==(Value<N> a, int b) { return a.value == b; }

template <int N> void exercise(unsigned& checksum, int input) {
    Value<N> value{input + N};
    std::string text = zeroerr::format("value {name}: {number}", value, N);
    zeroerr::ExprResult comparison = zeroerr::ExpressionDecomposer() << value == N;
    zeroerr::ExprResult combined = comparison && zeroerr::ExprResult(true, "true");
    for (unsigned char c : text) checksum = checksum * 33u + c;
    for (unsigned char c : combined.decomp) checksum = checksum * 33u + c;
}
template <int N> struct Workload {
    static void run(unsigned& sum, int input) {
        exercise<N>(sum, input);
        Workload<N - 1>::run(sum, input);
    }
};
template <> struct Workload<0> {
    static void run(unsigned& sum, int input) { exercise<0>(sum, input); }
};
int main(int argc, char**) {
    unsigned checksum = 0;
    Workload<23>::run(checksum, argc);
    std::cout << checksum << '\n';
}
