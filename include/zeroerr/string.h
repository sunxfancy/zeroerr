#pragma once

#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>

namespace zeroerr {
class StringBuffer;
namespace detail {
template <class T>
struct StringLike {
    template <class U>
    static auto test(int)
        -> decltype(static_cast<const char*>(std::declval<const U&>().data()),
                    static_cast<std::size_t>(std::declval<const U&>().size()), std::true_type{});
    template <class>
    static std::false_type test(...);
    static constexpr bool  value = decltype(test<T>(0))::value;
    template <class U>
    static auto own(int)
        -> decltype(std::declval<U&>().append(static_cast<const char*>(nullptr), std::size_t{}),
                    std::true_type{});
    template <class>
    static std::false_type own(...);
    static constexpr bool  owning = value && decltype(own<T>(0))::value;
};
}  // namespace detail


// Owning diagnostic text, independent of the standard string and stream headers.
class String {
public:
    using size_type                 = std::size_t;
    using value_type                = char;
    using iterator                  = char*;
    using const_iterator            = const char*;
    static constexpr size_type npos = static_cast<size_type>(-1);

    String() noexcept;
    String(const char* text);
    String(const char* text, size_type size);
    String(size_type count, char value);
    template <class T,
              typename std::enable_if<
                  detail::StringLike<T>::value && !std::is_same<T, String>::value, int>::type = 0>
    String(const T& text) : String(text.data(), text.size()) {}
    template <class T,
              typename std::enable_if<
                  detail::StringLike<T>::owning && !std::is_same<T, String>::value, int>::type = 0>
    operator T() const {
        return T(data_, size_);
    }
    template <class Iterator,
              typename std::enable_if<!std::is_integral<Iterator>::value, int>::type = 0>
    String(Iterator first, Iterator last) : String() {
        for (; first != last; ++first) push_back(*first);
    }
    String(const String& other);
    String(String&& other) noexcept;
    ~String();
    String& operator=(const String& other);
    String& operator=(String&& other) noexcept;
    String& operator=(const char* text);

    const char*    c_str() const noexcept { return data_; }
    const char*    data() const noexcept { return data_; }
    char*          data() noexcept { return data_; }
    size_type      size() const noexcept { return size_; }
    size_type      length() const noexcept { return size_; }
    bool           empty() const noexcept { return size_ == 0; }
    size_type      capacity() const noexcept { return capacity_; }
    iterator       begin() noexcept { return data_; }
    iterator       end() noexcept { return data_ + size_; }
    const_iterator begin() const noexcept { return data_; }
    const_iterator end() const noexcept { return data_ + size_; }
    char&          operator[](size_type i) noexcept { return data_[i]; }
    const char&    operator[](size_type i) const noexcept { return data_[i]; }
    char&          back() noexcept { return data_[size_ - 1]; }
    const char&    back() const noexcept { return data_[size_ - 1]; }

    void    clear() noexcept;
    void    reserve(size_type capacity);
    void    resize(size_type size, char value = '\0');
    String& append(const char* text, size_type size);
    String& append(const char* text);
    String& append(const String& text);
    void    push_back(char value);
    String& operator+=(const String& text) { return append(text); }
    String& operator+=(const char* text) { return append(text); }
    String& operator+=(char value) {
        push_back(value);
        return *this;
    }
    int       compare(const String& other) const noexcept;
    size_type find(const String& text, size_type pos = 0) const noexcept;
    size_type find(char value, size_type pos = 0) const noexcept;
    size_type find_first_of(char value, size_type pos = 0) const noexcept {
        return find(value, pos);
    }
    size_type find_last_of(char value, size_type pos = npos) const noexcept;
    size_type rfind(const String& text, size_type pos = npos) const noexcept;
    String    substr(size_type pos = 0, size_type count = npos) const;
    String&   replace(size_type pos, size_type count, const String& text);

private:
    char      local_[24];
    char*     data_;
    size_type size_;
    size_type capacity_;
};

inline bool operator==(const String& a, const String& b) noexcept { return a.compare(b) == 0; }
inline bool operator!=(const String& a, const String& b) noexcept { return a.compare(b) != 0; }
inline bool operator<(const String& a, const String& b) noexcept { return a.compare(b) < 0; }
inline bool operator>(const String& a, const String& b) noexcept { return a.compare(b) > 0; }
inline bool operator<=(const String& a, const String& b) noexcept { return a.compare(b) <= 0; }
inline bool operator>=(const String& a, const String& b) noexcept { return a.compare(b) >= 0; }
String      operator+(const String& a, const String& b);

template <class Stream, class T,
          typename std::enable_if<std::is_same<typename std::decay<T>::type, String>::value &&
                                      !std::is_same<Stream, StringBuffer>::value,
                                  int>::type = 0>
auto operator<<(Stream& stream, T&& text) -> decltype(stream.write(text.data(), text.size())) {
    return stream.write(text.data(), text.size());
}
template <class Stream>
Stream& operator>>(Stream& stream, String& text) {
    text.clear();
    int c = stream.peek();
    while (c != -1 && (c == ' ' || c == '\t' || c == '\n' || c == '\r')) {
        stream.get();
        c = stream.peek();
    }
    while (c != -1 && c != ' ' && c != '\t' && c != '\n' && c != '\r') {
        text.push_back(static_cast<char>(stream.get()));
        c = stream.peek();
    }
    if (text.empty()) stream.setstate(Stream::failbit);
    return stream;
}
template <class Stream>
bool getline(Stream& stream, String& text) {
    text.clear();
    char c;
    while (stream.get(c)) {
        if (c == '\n') return true;
        text.push_back(c);
    }
    return !text.empty();
}
}  // namespace zeroerr
namespace std {
template <>
struct hash<zeroerr::String> {
    size_t operator()(const zeroerr::String& value) const noexcept {
        size_t result = 2166136261u;
        for (char c : value) result = (result ^ static_cast<unsigned char>(c)) * 16777619u;
        return result;
    }
};
}  // namespace std
