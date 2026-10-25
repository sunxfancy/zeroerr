#pragma once

#include <cstddef>

#include "zeroerr/buffer.h"
#include "zeroerr/string.h"

// Avoid duplicating diagnostic setup in each caller. Keep equivalent template
// instantiations available for linker folding without changing user build flags.
#if defined(_MSC_VER)
#define ZEROERR_DIAGNOSTIC_NOINLINE __declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
#define ZEROERR_DIAGNOSTIC_NOINLINE __attribute__((noinline))
#else
#define ZEROERR_DIAGNOSTIC_NOINLINE
#endif

namespace zeroerr {
struct Printer;
namespace detail {

// Plain append-only storage. Pure concatenation does not construct an iostream,
// streambuf, locale, or virtual table; only value printing needs the adapter.
class TextBuffer {
public:
    TextBuffer();
    ~TextBuffer();
    TextBuffer(const TextBuffer&)            = delete;
    TextBuffer& operator=(const TextBuffer&) = delete;

    void        append(const char* data, std::size_t size);
    void        append(char value);
    const char* data() const;
    std::size_t size() const;
    zeroerr::String str() const;

private:
    char        local_[256];
    char*       data_;
    std::size_t size_;
    std::size_t capacity_;
};

class TextStream {
public:
    StringBuffer stream;
};

// Borrowed values are printed synchronously while their caller is alive. The
// callback preserves PrinterExt and arbitrary ostream insertion overloads.
struct DiagnosticValue {
    const void* object;
    void (*print)(Printer&, const void*);
};

zeroerr::String formatText(const char* fmt, const DiagnosticValue* values, std::size_t count,
                       std::size_t* ends);
zeroerr::String comparisonText(const zeroerr::String& prefix, DiagnosticValue lhs, const char* op,
                           DiagnosticValue rhs, bool& result);
zeroerr::String valueText(DiagnosticValue value);
zeroerr::String joinDiagnostic(const zeroerr::String& lhs, const char* op, const zeroerr::String& rhs);
zeroerr::String assertionText(const zeroerr::String& condition, const zeroerr::String& message,
                          const char* file, unsigned line);

}  // namespace detail
}  // namespace zeroerr
