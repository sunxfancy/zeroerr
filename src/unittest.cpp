#include "zeroerr/unittest.h"
#include "zeroerr/assert.h"
#include "zeroerr/color.h"

#include <iomanip>
#include <iostream>
#include <ostream>
#include <set>
#include <string>
#include <vector>

namespace zeroerr {

namespace detail {
static std::set<TestCase>& getRegisteredTests();
}

UnitTest& UnitTest::parseArgs(int argc, char** argv) { return *this; }

static inline std::string getFileName(std::string file) {
    std::string fileName(file);
    auto        p = fileName.find_last_of('/');
    if (p != std::string::npos) fileName = fileName.substr(p + 1);
    return fileName;
}

int UnitTest::run() {
    std::cout << "ZeroErr Unit Test";
    TestContext context;
    unsigned    passed = 0, warning = 0, failed = 0, skipped = 0;

    for (auto& tc : detail::getRegisteredTests()) {
        std::cout << std::endl
                  << "TEST CASE " << Dim << "[" << getFileName(tc.file) << ":" << tc.line << "] "
                  << Reset << FgCyan << tc.name << Reset << std::endl
                  << std::endl;
        try {
            tc.func(&context);  // run the test case
        } catch (const AssertionData& e) {
            failed++;
            continue;
        } catch (const std::exception& e) {
            failed++;
            continue;
        }
        passed++;
    }
    std::cout << "----------------------------------------------------------------" << std::endl;
    std::cout << "             " << FgGreen << "PASSED" << Reset << "   |   " << FgYellow
              << "WARNING" << Reset << "   |   " << FgRed << "FAILED" << Reset << "   |   " << Dim
              << "SKIPPED" << Reset << std::endl;
    std::cout << "TEST CASE:   " << std::setw(6) << passed << "       " << std::setw(7) << warning
              << "       " << std::setw(6) << failed << "       " << std::setw(7) << skipped
              << std::endl;
    std::cout << "ASSERTION: " << std::endl;
    return 0;
}

// sorted by file names and line numbers
bool TestCase::operator<(const TestCase& rhs) const {
    return (file < rhs.file) || (file == rhs.file && line < rhs.line);
}

namespace detail {

static std::set<TestCase>& getRegisteredTests() {
    static std::set<TestCase> data;
    return data;
}

regTest::regTest(const TestCase& tc) { getRegisteredTests().insert(tc); }

static std::set<IReporter*>& getRegisteredReporters() {
    static std::set<IReporter*> data;
    return data;
}

regReporter::regReporter(IReporter* reporter) { getRegisteredReporters().insert(reporter); }

}  // namespace detail


class ConsoleReporter : public IReporter {
public:
};


class XmlEncode;
class XmlWriter;

class XmlReporter : public IReporter {
public:
};

class JUnitReporter : public IReporter {
public:
};


// =================================================================================================
// The following code has been taken verbatim from Catch2/include/internal/catch_xmlwriter.h/cpp
// =================================================================================================
class XmlEncode {
public:
    enum ForWhat { ForTextNodes, ForAttributes };

    XmlEncode(std::string const& str, ForWhat forWhat = ForTextNodes);

    void encodeTo(std::ostream& os) const;

    friend std::ostream& operator<<(std::ostream& os, XmlEncode const& xmlEncode);

private:
    std::string m_str;
    ForWhat     m_forWhat;
};

class XmlWriter {
public:
    class ScopedElement {
    public:
        ScopedElement(XmlWriter* writer);

        ScopedElement(ScopedElement&& other) noexcept;
        ScopedElement& operator=(ScopedElement&& other) noexcept;

        ~ScopedElement();

        ScopedElement& writeText(std::string const& text, bool indent = true);

        template <typename T>
        ScopedElement& writeAttribute(std::string const& name, T const& attribute) {
            m_writer->writeAttribute(name, attribute);
            return *this;
        }

    private:
        mutable XmlWriter* m_writer = nullptr;
    };

    XmlWriter(std::ostream& os = std::cout);
    ~XmlWriter();

    XmlWriter(XmlWriter const&)            = delete;
    XmlWriter& operator=(XmlWriter const&) = delete;

    XmlWriter& startElement(std::string const& name);

    ScopedElement scopedElement(std::string const& name);

    XmlWriter& endElement();

    XmlWriter& writeAttribute(std::string const& name, std::string const& attribute);

    XmlWriter& writeAttribute(std::string const& name, const char* attribute);

    XmlWriter& writeAttribute(std::string const& name, bool attribute);

    template <typename T>
    XmlWriter& writeAttribute(std::string const& name, T const& attribute) {
        std::stringstream rss;
        rss << attribute;
        return writeAttribute(name, rss.str());
    }

    XmlWriter& writeText(std::string const& text, bool indent = true);

    void ensureTagClosed();

    void writeDeclaration();

private:
    void newlineIfNecessary();

    bool                     m_tagIsOpen    = false;
    bool                     m_needsNewline = false;
    std::vector<std::string> m_tags;
    std::string              m_indent;
    std::ostream&            m_os;
};

using uchar = unsigned char;

static size_t trailingBytes(unsigned char c) {
    if ((c & 0xE0) == 0xC0) {
        return 2;
    }
    if ((c & 0xF0) == 0xE0) {
        return 3;
    }
    if ((c & 0xF8) == 0xF0) {
        return 4;
    }
    throw std::runtime_error("Invalid multibyte utf-8 start byte encountered");
}

static uint32_t headerValue(unsigned char c) {
    if ((c & 0xE0) == 0xC0) {
        return c & 0x1F;
    }
    if ((c & 0xF0) == 0xE0) {
        return c & 0x0F;
    }
    if ((c & 0xF8) == 0xF0) {
        return c & 0x07;
    }
    throw std::runtime_error("Invalid multibyte utf-8 start byte encountered");
}

static void hexEscapeChar(std::ostream& os, unsigned char c) {
    std::ios_base::fmtflags f(os.flags());
    os << "\\x" << std::uppercase << std::hex << std::setfill('0') << std::setw(2)
       << static_cast<int>(c);
    os.flags(f);
}

XmlEncode::XmlEncode(std::string const& str, ForWhat forWhat) : m_str(str), m_forWhat(forWhat) {}

void XmlEncode::encodeTo(std::ostream& os) const {
    // Apostrophe escaping not necessary if we always use " to write attributes
    // (see: https://www.w3.org/TR/xml/#syntax)

    for (std::size_t idx = 0; idx < m_str.size(); ++idx) {
        uchar c = m_str[idx];
        switch (c) {
            case '<': os << "&lt;"; break;
            case '&': os << "&amp;"; break;

            case '>':
                // See: https://www.w3.org/TR/xml/#syntax
                if (idx > 2 && m_str[idx - 1] == ']' && m_str[idx - 2] == ']')
                    os << "&gt;";
                else
                    os << c;
                break;

            case '\"':
                if (m_forWhat == ForAttributes)
                    os << "&quot;";
                else
                    os << c;
                break;

            default:
                // Check for control characters and invalid utf-8

                // Escape control characters in standard ascii
                // see
                // https://stackoverflow.com/questions/404107/why-are-control-characters-illegal-in-xml-1-0
                if (c < 0x09 || (c > 0x0D && c < 0x20) || c == 0x7F) {
                    hexEscapeChar(os, c);
                    break;
                }

                // Plain ASCII: Write it to stream
                if (c < 0x7F) {
                    os << c;
                    break;
                }

                // UTF-8 territory
                // Check if the encoding is valid and if it is not, hex escape bytes.
                // Important: We do not check the exact decoded values for validity, only the
                // encoding format First check that this bytes is a valid lead byte: This means that
                // it is not encoded as 1111 1XXX Or as 10XX XXXX
                if (c < 0xC0 || c >= 0xF8) {
                    hexEscapeChar(os, c);
                    break;
                }

                auto encBytes = trailingBytes(c);
                // Are there enough bytes left to avoid accessing out-of-bounds memory?
                if (idx + encBytes - 1 >= m_str.size()) {
                    hexEscapeChar(os, c);
                    break;
                }
                // The header is valid, check data
                // The next encBytes bytes must together be a valid utf-8
                // This means: bitpattern 10XX XXXX and the extracted value is sane (ish)
                bool     valid = true;
                uint32_t value = headerValue(c);
                for (std::size_t n = 1; n < encBytes; ++n) {
                    uchar nc = m_str[idx + n];
                    valid &= ((nc & 0xC0) == 0x80);
                    value = (value << 6) | (nc & 0x3F);
                }

                if (
                    // Wrong bit pattern of following bytes
                    (!valid) ||
                    // Overlong encodings
                    (value < 0x80) ||
                    (value < 0x800 &&
                     encBytes > 2) ||  // removed "0x80 <= value &&" because redundant
                    (0x800 < value && value < 0x10000 && encBytes > 3) ||
                    // Encoded value out of range
                    (value >= 0x110000)) {
                    hexEscapeChar(os, c);
                    break;
                }

                // If we got here, this is in fact a valid(ish) utf-8 sequence
                for (std::size_t n = 0; n < encBytes; ++n) {
                    os << m_str[idx + n];
                }
                idx += encBytes - 1;
                break;
        }
    }
}

std::ostream& operator<<(std::ostream& os, XmlEncode const& xmlEncode) {
    xmlEncode.encodeTo(os);
    return os;
}

XmlWriter::ScopedElement::ScopedElement(XmlWriter* writer) : m_writer(writer) {}

XmlWriter::ScopedElement::ScopedElement(ScopedElement&& other) noexcept : m_writer(other.m_writer) {
    other.m_writer = nullptr;
}

XmlWriter::ScopedElement& XmlWriter::ScopedElement::operator=(ScopedElement&& other) noexcept {
    if (m_writer) {
        m_writer->endElement();
    }
    m_writer       = other.m_writer;
    other.m_writer = nullptr;
    return *this;
}


XmlWriter::ScopedElement::~ScopedElement() {
    if (m_writer) m_writer->endElement();
}

XmlWriter::ScopedElement& XmlWriter::ScopedElement::writeText(std::string const& text,
                                                              bool               indent) {
    m_writer->writeText(text, indent);
    return *this;
}

XmlWriter::XmlWriter(std::ostream& os) : m_os(os) {}

XmlWriter::~XmlWriter() {
    while (!m_tags.empty()) endElement();
}

XmlWriter& XmlWriter::startElement(std::string const& name) {
    ensureTagClosed();
    newlineIfNecessary();
    m_os << m_indent << '<' << name;
    m_tags.push_back(name);
    m_indent += "  ";
    m_tagIsOpen = true;
    return *this;
}

XmlWriter::ScopedElement XmlWriter::scopedElement(std::string const& name) {
    ScopedElement scoped(this);
    startElement(name);
    return scoped;
}

XmlWriter& XmlWriter::endElement() {
    newlineIfNecessary();
    m_indent = m_indent.substr(0, m_indent.size() - 2);
    if (m_tagIsOpen) {
        m_os << "/>";
        m_tagIsOpen = false;
    } else {
        m_os << m_indent << "</" << m_tags.back() << ">";
    }
    m_os << std::endl;
    m_tags.pop_back();
    return *this;
}

XmlWriter& XmlWriter::writeAttribute(std::string const& name, std::string const& attribute) {
    if (!name.empty() && !attribute.empty())
        m_os << ' ' << name << "=\"" << XmlEncode(attribute, XmlEncode::ForAttributes) << '"';
    return *this;
}

XmlWriter& XmlWriter::writeAttribute(std::string const& name, const char* attribute) {
    if (!name.empty() && attribute && attribute[0] != '\0')
        m_os << ' ' << name << "=\"" << XmlEncode(attribute, XmlEncode::ForAttributes) << '"';
    return *this;
}

XmlWriter& XmlWriter::writeAttribute(std::string const& name, bool attribute) {
    m_os << ' ' << name << "=\"" << (attribute ? "true" : "false") << '"';
    return *this;
}

XmlWriter& XmlWriter::writeText(std::string const& text, bool indent) {
    if (!text.empty()) {
        bool tagWasOpen = m_tagIsOpen;
        ensureTagClosed();
        if (tagWasOpen && indent) m_os << m_indent;
        m_os << XmlEncode(text);
        m_needsNewline = true;
    }
    return *this;
}

void XmlWriter::ensureTagClosed() {
    if (m_tagIsOpen) {
        m_os << ">" << std::endl;
        m_tagIsOpen = false;
    }
}

void XmlWriter::writeDeclaration() { m_os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"; }

void XmlWriter::newlineIfNecessary() {
    if (m_needsNewline) {
        m_os << std::endl;
        m_needsNewline = false;
    }
}

// =================================================================================================
// End of copy-pasted code from Catch
// =================================================================================================


}  // namespace zeroerr


int main(int argc, char** argv) { return zeroerr::UnitTest().parseArgs(argc, argv).run(); }
