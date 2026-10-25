#pragma once

// Explicit opt-in to the standard string/iostream backend. Core headers never
// auto-enable it based on which standard headers happen to have been included.
#include <ostream>
#include <sstream>
#include <string>
#include "zeroerr/print.h"

namespace zeroerr {
inline std::ostream& operator<<(std::ostream& stream, const Printer& printer) {
    if (printer.use_stringstream) {
        const auto text = printer.str();
        stream.write(text.data(), static_cast<std::streamsize>(text.size()));
    }
    return stream;
}
namespace detail {
inline void applyFormat(std::ostream& stream, const StringBuffer::Format& f) {
    stream.precision(f.precision);
    stream.width(f.width);
    stream.fill(f.fill);
    if (f.base == 16)
        stream << std::hex;
    else if (f.base == 8)
        stream << std::oct;
    else
        stream << std::dec;
    if (f.style == 1)
        stream << std::fixed;
    else if (f.style == 2)
        stream << std::scientific;
}
inline void saveFormat(StringBuffer& buffer, const std::ostream& stream) {
    auto& f     = buffer.format();
    auto  flags = stream.flags();
    f.base      = (flags & std::ios_base::basefield) == std::ios_base::hex   ? 16
                  : (flags & std::ios_base::basefield) == std::ios_base::oct ? 8
                                                                             : 10;
    f.style     = (flags & std::ios_base::floatfield) == std::ios_base::fixed        ? 1
                  : (flags & std::ios_base::floatfield) == std::ios_base::scientific ? 2
                                                                                     : 0;
    f.precision = static_cast<int>(stream.precision());
    f.width     = static_cast<int>(stream.width());
    f.fill      = stream.fill();
}
template <class T>
StringBuffer& standardInsert(StringBuffer& buffer, const T& value) {
    std::ostringstream stream;
    applyFormat(stream, buffer.format());
    stream << value;
    saveFormat(buffer, stream);
    const auto text = stream.str();
    return buffer.write(text.data(), text.size());
}
}  // namespace detail
inline StringBuffer& operator<<(StringBuffer& buffer, std::ios_base& (*manip)(std::ios_base&)) {
    return detail::standardInsert(buffer, manip);
}
inline StringBuffer& operator<<(StringBuffer& buffer, std::ostream& (*manip)(std::ostream&)) {
    detail::standardInsert(buffer, manip);
    buffer.flush();
    return buffer;
}
template <class T>
typename std::enable_if<std::is_class<T>::value && !detail::is_string<T>::value &&
                            detail::is_streamable<std::ostream, T>::value,
                        StringBuffer&>::type
operator<<(StringBuffer& buffer, const T& value) {
    return detail::standardInsert(buffer, value);
}

template <class T>
typename std::enable_if<std::is_class<typename std::decay<T>::type>::value && !detail::is_string<typename std::decay<T>::type>::value &&
                            !detail::is_container<typename std::decay<T>::type>::value && !detail::is_complex<typename std::decay<T>::type>::value &&
                            detail::is_streamable<std::ostream, T>::value,
                        void>::type
PrinterExt(Printer& printer, T&& value, unsigned level, const char* lb, rank<1>) {
    printer.os << printer.tab(level);
    detail::standardInsert(printer.os, value);
    printer.os << lb;
}
}  // namespace zeroerr
