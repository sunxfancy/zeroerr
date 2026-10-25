#include "zeroerr/buffer.h"
#include <cstdio>

namespace zeroerr {
StringBuffer& StringBuffer::write(const char* data, std::size_t size) {
    if (write_)
        write_(target_, data, size);
    else
        text_.append(data, size);
    return *this;
}
void StringBuffer::flush() {
    if (flush_) flush_(target_);
}
StringBuffer& StringBuffer::operator<<(const char* text) {
    std::size_t size = 0;
    while (text[size]) ++size;
    return padded(text, size);
}
StringBuffer& StringBuffer::padded(const char* text, std::size_t size) {
    for (int i = static_cast<int>(size); i < format_.width; ++i) write(&format_.fill, 1);
    format_.width = 0;
    return write(text, size);
}
StringBuffer& StringBuffer::integer(unsigned long long value, bool negative) {
    char     text[66];
    char*    end  = text + sizeof(text);
    char*    p    = end;
    unsigned base = static_cast<unsigned>(format_.base);
    if (base != 8 && base != 16) base = 10;
    if (negative && base == 10) value = 0 - value;
    do {
        *--p = "0123456789abcdef"[value % base];
        value /= base;
    } while (value);
    if (negative && base == 10) *--p = '-';
    return padded(p, static_cast<std::size_t>(end - p));
}
StringBuffer& StringBuffer::floating(long double value) {
    const char* format = format_.style == 1 ? "%.*Lf" : format_.style == 2 ? "%.*Le" : "%.*Lg";
    int         size   = std::snprintf(nullptr, 0, format, format_.precision, value);
    if (size < 0) return *this;
    String text(static_cast<std::size_t>(size), '\0');
    std::snprintf(text.data(), text.size() + 1, format, format_.precision, value);
    return padded(text.data(), text.size());
}
StringBuffer& StringBuffer::operator<<(const void* value) {
    char text[2 * sizeof(void*) + 4];
    int  size = std::snprintf(text, sizeof(text), "%p", value);
    if (size > 0) padded(text, static_cast<std::size_t>(size));
    return *this;
}
}  // namespace zeroerr
