#pragma once
#include <type_traits>
#include "zeroerr/string.h"

namespace zeroerr {

class StringBuffer {
public:
    struct Format {
        int  base      = 10;
        int  precision = 6;
        int  width     = 0;
        char fill      = ' ';
        int  style     = 0;
    };
    StringBuffer() = default;
    template <class Stream>
    void attach(Stream& stream) {
        target_ = &stream;
        write_  = [](void* p, const char* data, std::size_t size) {
            static_cast<Stream*>(p)->write(data, size);
        };
        flush_ = [](void* p) { static_cast<Stream*>(p)->flush(); };
    }
    template <class Stream,
              typename std::enable_if<!std::is_same<Stream, StringBuffer>::value, int>::type = 0>
    explicit StringBuffer(Stream& stream)
        : target_(&stream),
          write_([](void* p, const char* data, std::size_t size) {
              static_cast<Stream*>(p)->write(data, size);
          }),
          flush_([](void* p) { static_cast<Stream*>(p)->flush(); }) {}
    StringBuffer& write(const char* data, std::size_t size);
    void          flush();
    void          clear() { text_.clear(); }
    String        str() const { return text_; }
    const String& text() const { return text_; }
    Format&       format() { return format_; }
    const Format& format() const { return format_; }
    StringBuffer& operator<<(const String& text) { return padded(text.data(), text.size()); }
    StringBuffer& operator<<(const char* text);
    StringBuffer& operator<<(char value) { return write(&value, 1); }
    StringBuffer& operator<<(const void* value);
    StringBuffer& operator<<(std::nullptr_t) { return *this << static_cast<const void*>(nullptr); }
    template <class T>
    typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, char>::value,
                            StringBuffer&>::type
    operator<<(T value) {
        unsigned long long bits = static_cast<unsigned long long>(value);
        if (format_.base == 8 || format_.base == 16)
            bits &= (~0ull >> ((sizeof(unsigned long long) - sizeof(T)) * 8));
        return integer(bits, std::is_signed<T>::value && value < 0);
    }
    template <class T>
    typename std::enable_if<std::is_floating_point<T>::value, StringBuffer&>::type operator<<(
        T value) {
        return floating(static_cast<long double>(value));
    }
    template <class T>
    typename std::enable_if<std::is_enum<T>::value, StringBuffer&>::type operator<<(T value) {
        return *this << static_cast<typename std::underlying_type<T>::type>(value);
    }

private:
    StringBuffer& integer(unsigned long long value, bool negative);
    StringBuffer& floating(long double value);
    StringBuffer& padded(const char* text, std::size_t size);
    String        text_;
    void*         target_                           = nullptr;
    void (*write_)(void*, const char*, std::size_t) = nullptr;
    void (*flush_)(void*)                           = nullptr;
    Format format_;
};

}  // namespace zeroerr
