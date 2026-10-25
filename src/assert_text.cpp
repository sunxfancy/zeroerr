
#include "zeroerr/color.h"
#include "zeroerr/internal/text.h"

namespace zeroerr {
namespace detail {

zeroerr::String assertionText(const zeroerr::String& condition, const zeroerr::String& message,
                          const char* file, unsigned line) {
    TextStream stream;
    stream.stream << "    ";
    stream.stream.write(condition.data(), condition.size());
    stream.stream << "  expands to  ";
    stream.stream.write(message.data(), message.size());
    stream.stream << '\n' << Dim << '(' << file << ':' << line << ')' << Reset << '\n';
    return stream.stream.str();
}

}  // namespace detail
}  // namespace zeroerr
