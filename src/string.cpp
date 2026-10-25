#include "zeroerr/string.h"

#include <cstring>
#include <stdexcept>

namespace zeroerr {

constexpr String::size_type String::npos;

String::String() noexcept : data_(local_), size_(0), capacity_(sizeof(local_) - 1) {
    local_[0] = '\0';
}
String::String(const char* text) : String(text, std::strlen(text)) {}
String::String(const char* text, size_type size) : String() { append(text, size); }
String::String(size_type count, char value) : String() { resize(count, value); }
String::String(const String& other) : String(other.data_, other.size_) {}
String::String(String&& other) noexcept : String() { *this = static_cast<String&&>(other); }
String::~String() {
    if (data_ != local_) delete[] data_;
}

String& String::operator=(String&& other) noexcept {
    if (this == &other) return *this;
    if (data_ != local_) delete[] data_;
    if (other.data_ == other.local_) {
        data_     = local_;
        capacity_ = sizeof(local_) - 1;
        std::memcpy(data_, other.data_, other.size_ + 1);
    } else {
        data_     = other.data_;
        capacity_ = other.capacity_;
    }
    size_           = other.size_;
    other.data_     = other.local_;
    other.capacity_ = sizeof(other.local_) - 1;
    other.clear();
    return *this;
}
String& String::operator=(const String& other) {
    if (this != &other) {
        String copy(other);
        *this = static_cast<String&&>(copy);
    }
    return *this;
}
String& String::operator=(const char* text) {
    String copy(text);
    return *this = static_cast<String&&>(copy);
}
void String::clear() noexcept {
    size_    = 0;
    data_[0] = '\0';
}
void String::reserve(size_type capacity) {
    if (capacity <= capacity_) return;
    if (capacity == npos) throw std::length_error("zeroerr::String too long");
    char* next = new char[capacity + 1];
    std::memcpy(next, data_, size_ + 1);
    if (data_ != local_) delete[] data_;
    data_     = next;
    capacity_ = capacity;
}
void String::resize(size_type size, char value) {
    reserve(size);
    if (size > size_) std::memset(data_ + size_, value, size - size_);
    size_        = size;
    data_[size_] = '\0';
}
String& String::append(const char* text, size_type size) {
    if (!size) return *this;
    if (size > npos - size_ - 1) throw std::length_error("zeroerr::String too long");
    size_type required = size_ + size;
    if (required > capacity_) {
        size_type grown    = capacity_ <= (npos - 1) / 2 ? capacity_ * 2 : npos - 1;
        size_type capacity = required > grown ? required : grown;
        char*     next     = new char[capacity + 1];
        std::memcpy(next, data_, size_);
        // Read aliased input before releasing the old allocation.
        std::memcpy(next + size_, text, size);
        if (data_ != local_) delete[] data_;
        data_     = next;
        capacity_ = capacity;
    } else {
        std::memmove(data_ + size_, text, size);
    }
    size_        = required;
    data_[size_] = '\0';
    return *this;
}
String& String::append(const char* text) { return append(text, std::strlen(text)); }
String& String::append(const String& text) { return append(text.data_, text.size_); }
void    String::push_back(char value) { append(&value, 1); }
int     String::compare(const String& other) const noexcept {
    const size_type common = size_ < other.size_ ? size_ : other.size_;
    const int       result = std::memcmp(data_, other.data_, common);
    return result ? result : (size_ < other.size_ ? -1 : size_ > other.size_ ? 1 : 0);
}
String::size_type String::find(const String& text, size_type pos) const noexcept {
    if (pos > size_ || text.size_ > size_ - pos) return npos;
    for (size_type i = pos; i <= size_ - text.size_; ++i)
        if (!std::memcmp(data_ + i, text.data_, text.size_)) return i;
    return npos;
}
String::size_type String::find(char value, size_type pos) const noexcept {
    for (size_type i = pos; i < size_; ++i)
        if (data_[i] == value) return i;
    return npos;
}
String::size_type String::find_last_of(char value, size_type pos) const noexcept {
    size_type i = pos < size_ ? pos + 1 : size_;
    while (i) {
        --i;
        if (data_[i] == value) return i;
    }
    return npos;
}
String::size_type String::rfind(const String& text, size_type pos) const noexcept {
    if (text.size_ > size_) return npos;
    size_type i = size_ - text.size_;
    if (pos < i) i = pos;
    for (;;) {
        if (!std::memcmp(data_ + i, text.data_, text.size_)) return i;
        if (!i) return npos;
        --i;
    }
}
String String::substr(size_type pos, size_type count) const {
    if (pos > size_) throw std::out_of_range("zeroerr::String::substr");
    if (count > size_ - pos) count = size_ - pos;
    return String(data_ + pos, count);
}
String& String::replace(size_type pos, size_type count, const String& text) {
    if (pos > size_) throw std::out_of_range("zeroerr::String::replace");
    if (count > size_ - pos) count = size_ - pos;
    String result(data_, pos);
    result.append(text);
    result.append(data_ + pos + count, size_ - pos - count);
    *this = static_cast<String&&>(result);
    return *this;
}
String operator+(const String& a, const String& b) {
    String result(a);
    result.append(b);
    return result;
}

}  // namespace zeroerr
