
#include "zeroerr/internal/text.h"
#include "zeroerr/print.h"

#include <cstring>
#include <limits>
#include <stdexcept>

namespace zeroerr {

zeroerr::String Printer::tab(unsigned level) {
    return zeroerr::String(isCompact ? 0 : level * indent, ' ');
}

namespace detail {

TextBuffer::TextBuffer() : data_(local_), size_(0), capacity_(sizeof(local_)) { data_[0] = '\0'; }
TextBuffer::~TextBuffer() {
    if (data_ != local_) delete[] data_;
}

void TextBuffer::append(const char* source, std::size_t count) {
    if (count == 0) return;
    const std::size_t limit = (std::numeric_limits<std::size_t>::max)();
    if (count > limit - size_ - 1) throw std::length_error("zeroerr diagnostic text too long");
    const std::size_t required = size_ + count + 1;
    if (required > capacity_) {
        const std::size_t grown    = capacity_ <= limit / 2 ? capacity_ * 2 : limit;
        const std::size_t capacity = required > grown ? required : grown;
        char*             next     = new char[capacity];
        std::memcpy(next, data_, size_);
        // Read source before releasing the old allocation, including when
        // appending a slice of our own buffer.
        std::memcpy(next + size_, source, count);
        if (data_ != local_) delete[] data_;
        data_     = next;
        capacity_ = capacity;
    } else {
        std::memmove(data_ + size_, source, count);
    }
    size_ += count;
    data_[size_] = '\0';
}

void        TextBuffer::append(char value) { append(&value, 1); }
const char* TextBuffer::data() const { return data_; }
std::size_t TextBuffer::size() const { return size_; }
zeroerr::String TextBuffer::str() const { return zeroerr::String(data_, size_); }

// No stream or Printer destructor is instantiated in the calling templates.
class MessageBuilder {
public:
    explicit MessageBuilder(bool quoted = true) : print(stream.stream) {
        print.isCompact  = true;
        print.isQuoted   = quoted;
        print.line_break = "";
    }
    void       value(DiagnosticValue value) { value.print(print, value.object); }
    TextStream stream;
    Printer    print;
};

zeroerr::String valueText(DiagnosticValue value) {
    MessageBuilder builder;
    builder.value(value);
    return builder.stream.stream.str();
}

zeroerr::String comparisonText(const zeroerr::String& prefix, DiagnosticValue lhs, const char* op,
                           DiagnosticValue rhs, bool& result) {
    MessageBuilder builder;
    if (prefix.empty()) {
        builder.value(lhs);
        result = true;
    } else {
        builder.stream.stream.write(prefix.data(), prefix.size());
    }
    builder.stream.stream.write(op, std::strlen(op));
    builder.value(rhs);
    return builder.stream.stream.str();
}

zeroerr::String formatText(const char* fmt, const DiagnosticValue* values, std::size_t count,
                       std::size_t* ends) {
    MessageBuilder builder(false);
    ends[0] = 0;
    for (std::size_t i = 0; i < count; ++i) {
        builder.value(values[i]);
        ends[i + 1] = builder.stream.stream.text().size();
    }
    TextBuffer  result;
    bool        parse_name = false;
    std::size_t argument   = 0;
    const char* literal    = fmt;
    for (const char* i = fmt; *i; ++i) {
        if (*i == '{') {
            if (!parse_name) result.append(literal, static_cast<std::size_t>(i - literal));
            parse_name = true;
        } else if (*i == '}') {
            if (!parse_name) result.append(literal, static_cast<std::size_t>(i - literal));
            if (argument == count)
                throw std::invalid_argument("zeroerr::format: too few arguments");
            result.append(builder.stream.stream.text().data() + ends[argument],
                          ends[argument + 1] - ends[argument]);
            ++argument;
            parse_name = false;
            literal    = i + 1;
        }
    }
    if (!parse_name) result.append(literal, std::strlen(literal));
    return result.str();
}

zeroerr::String joinDiagnostic(const zeroerr::String& lhs, const char* op, const zeroerr::String& rhs) {
    TextBuffer result;
    result.append(lhs.data(), lhs.size());
    result.append(op, std::strlen(op));
    result.append(rhs.data(), rhs.size());
    return result.str();
}

}  // namespace detail
}  // namespace zeroerr
