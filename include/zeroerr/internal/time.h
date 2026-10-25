#pragma once
namespace zeroerr {
struct Duration {
    double    seconds = 0;
    double    count() const noexcept { return seconds; }
    Duration& operator+=(Duration other) {
        seconds += other.seconds;
        return *this;
    }
    template <class T>
    bool operator>(const T& other) const {
        return seconds > other.count();
    }
};
struct Timestamp {
    long long nanoseconds = 0;
};
}  // namespace zeroerr
