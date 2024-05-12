#include "zeroerr/unittest.h"
#include "zeroerr/assert.h"
#include "zeroerr/color.h"
#include "zeroerr/fuzztest.h"
#include "zeroerr/internal/threadsafe.h"

#include <iomanip>
#include <iostream>
#include <ostream>
#include <regex>
#include <set>
#include <string>
#include <vector>

namespace zeroerr {

namespace detail {
static std::set<TestCase> getRegisteredTests(unsigned type);
}  // namespace detail

// This function update both sum and local.
// Local need to be updated since the reporter needs to know the result of the subcase.
int TestContext::add(TestContext& local) {
    int type = 0;
    if (local.failed_as == 0 && local.warning_as == 0) {
        passed += 1;
        local.passed += 1;
    } else if (local.failed_as == 0) {
        warning += 1;
        local.warning += 1;
        type = 1;
    } else {
        failed += 1;
        local.failed += 1;
        type = 2;
    }
    passed_as += local.passed_as;
    warning_as += local.warning_as;
    failed_as += local.failed_as;

    return type;
}

void TestContext::save_output() {
    std::fstream file;
    file.open("output.txt", std::ios::in);
    std::stringbuf* outbuf = static_cast<std::stringbuf*>(std::cerr.rdbuf());
    if (file.is_open()) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        if (buffer.str() != outbuf->str()) {
            std::cerr << "Output mismatch" << std::endl;
            throw std::runtime_error("Output mismatch");
        } else {
            std::cerr << "Output match" << std::endl;
        }
    } else {
        file.open("output.txt", std::ios::out);
        file << outbuf->str();
    }
    file.close();
}

void TestContext::reset() {
    passed = warning = failed = skipped = 0;
    passed_as = warning_as = failed_as = skipped_as = 0;
}

static inline std::string getFileName(std::string file) {
    std::string fileName(file);
    auto        p = fileName.find_last_of('/');
    if (p == std::string::npos) p = fileName.find_last_of('\\');
    if (p != std::string::npos) fileName = fileName.substr(p + 1);
    return fileName;
}

SubCase::SubCase(std::string name, std::string file, unsigned line, TestContext* context)
    : TestCase(name, file, line), context(context) {}

void SubCase::operator<<(std::function<void(TestContext*)> op) {
    func = op;
    std::stringbuf new_buf;
    context->reporter.subCaseStart(*this, new_buf);
    TestContext     local(context->reporter);
    std::streambuf* orig_buf = std::cerr.rdbuf();
    std::cerr.rdbuf(&new_buf);
    try {
        op(&local);
    } catch (const AssertionData&) {
    } catch (const FuzzFinishedException& e) {
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        if (local.failed_as == 0) {
            local.failed_as = 1;
        }
    }
    std::cerr.rdbuf(orig_buf);
    int type = context->add(local);

    context->reporter.subCaseEnd(*this, new_buf, local, type);
}

struct Filiters {
    std::vector<std::regex> name, name_exclude;
    std::vector<std::regex> file, file_exclude;
};

UnitTest& UnitTest::parseArgs(int argc, const char** argv) {
    filiters            = new Filiters();
    auto convert_to_vec = [=]() {
        std::vector<std::string> result;
        for (int i = 1; i < argc; i++) {
            result.emplace_back(argv[i]);
        }
        return result;
    };

    auto parse_char = [&](char arg) {
        if (arg == 'v') {
            this->silent = false;
            return true;
        }
        if (arg == 'q') {
            this->silent = true;
            return true;
        }
        if (arg == 'b') {
            this->run_bench = true;
            return true;
        }
        if (arg == 'f') {
            this->run_fuzz = true;
            return true;
        }
        if (arg == 'l') {
            this->list_test_cases = true;
            return true;
        }
        if (arg == 'x') {
            this->reporter_name = "xml";
            return true;
        }
        return false;
    };

    auto parse_token = [&](std::string arg) {
        if (arg == "verbose") {
            this->silent = false;
            return true;
        }
        if (arg == "quiet") {
            this->silent = true;
            return true;
        }
        if (arg == "bench") {
            this->run_bench = true;
        }
        if (arg == "fuzz") {
            this->run_fuzz = true;
        }
        if (arg == "list-test-cases") {
            this->list_test_cases = true;
        }
        if (arg.substr(0, 9) == "reporters") {
            this->reporter_name = arg.substr(10);
            return true;
        }
        if (arg.substr(0, 8) == "testcase") {
            filiters->name.push_back(std::regex(arg.substr(9)));
            return true;
        }
        if (arg.substr(0, 14) == "testcase-exclude") {
            filiters->name_exclude.push_back(std::regex(arg.substr(15)));
            return true;
        }
        if (arg.substr(0, 5) == "file") {
            filiters->file.push_back(std::regex(arg.substr(6)));
            return true;
        }
        if (arg.substr(0, 11) == "file-exclude") {
            filiters->file_exclude.push_back(std::regex(arg.substr(12)));
            return true;
        }
        return false;
    };

    auto parse_pos = [&](const std::vector<std::string>& args, size_t pos) {
        if (args[pos].size() == 2 && args[pos][0] == '-') {
            return parse_char(args[pos][1]);
        }
        if (args[pos].size() > 2 && args[pos][0] == '-' && args[pos][1] == '-') {
            return parse_token(args[pos].substr(2));
        }
        return false;
    };

    auto args = convert_to_vec();
    for (size_t i = 0; i < args.size(); ++i) parse_pos(args, i);

    binary = argv[0];
    return *this;
}


static std::string insertIndentation(std::string str) {
    std::stringstream result;
    std::stringstream ss(str);

    std::string line;
    while (std::getline(ss, line)) {
        result << line << std::endl << "    ";
    }

    return result.str();
}

bool UnitTest::run_filiter(const TestCase& tc) {
    if (filiters == nullptr) return true;
    for (auto& r : filiters->name)
        if (!std::regex_match(tc.name, r)) return false;
    for (auto& r : filiters->name_exclude)
        if (std::regex_match(tc.name, r)) return false;
    for (auto& r : filiters->file)
        if (!std::regex_match(tc.file, r)) return false;
    for (auto& r : filiters->file_exclude)
        if (std::regex_match(tc.file, r)) return false;
    return true;
}

int UnitTest::run() {
    IReporter* reporter = IReporter::create(reporter_name, *this);
    if (!reporter) reporter = IReporter::create("console", *this);
    TestContext context(*reporter), sum(*reporter);
    reporter->testStart();
    std::stringbuf new_buf;

    unsigned types = TestType::test_case;
    if (run_bench) types |= TestType::bench;
    if (run_fuzz) types |= TestType::fuzz_test;
    std::set<TestCase> testcases = detail::getRegisteredTests(types);

    for (auto& tc : testcases) {
        if (!run_filiter(tc)) continue;
        reporter->testCaseStart(tc, new_buf);
        if (!list_test_cases) {
            std::streambuf* orig_buf = std::cerr.rdbuf();
            std::cerr.rdbuf(&new_buf);
            std::cerr << std::endl;
            try {
                tc.func(&context);  // run the test case
            } catch (const AssertionData&) {
            } catch (const FuzzFinishedException& e) {
            } catch (const std::exception& e) {
                std::cerr << e.what() << std::endl;
                if (context.failed_as == 0) {
                    context.failed_as = 1;
                }
            }
            std::cerr.rdbuf(orig_buf);
        }
        int type = sum.add(context);
        reporter->testCaseEnd(tc, new_buf, context, type);
        context.reset();
        new_buf.str("");
    }
    reporter->testEnd(sum);
    delete reporter;
    return 0;
}

// sorted by file names and line numbers
bool TestCase::operator<(const TestCase& rhs) const {
    return (file < rhs.file) || (file == rhs.file && line < rhs.line);
}

namespace detail {

std::set<TestCase>& getTestSet(TestType type) {
    static std::set<TestCase> test_set, bench_set, fuzz_set;
    switch (type) {
        case TestType::test_case: return test_set;
        case TestType::bench:     return bench_set;
        case TestType::fuzz_test: return fuzz_set;
        case TestType::sub_case:  return test_set;
    }
}

static std::set<TestCase> getRegisteredTests(unsigned type) {
    std::set<TestCase> result;
    if (type & TestType::test_case)
        result.insert(getTestSet(test_case).begin(), getTestSet(test_case).end());
    if (type & TestType::bench) result.insert(getTestSet(bench).begin(), getTestSet(bench).end());
    if (type & TestType::fuzz_test)
        result.insert(getTestSet(fuzz_test).begin(), getTestSet(fuzz_test).end());
    return result;
}

regTest::regTest(const TestCase& tc, TestType type) { getTestSet(type).insert(tc); }

static std::set<IReporter*>& getRegisteredReporters() {
    static std::set<IReporter*> data;
    return data;
}

regReporter::regReporter(IReporter* reporter) { getRegisteredReporters().insert(reporter); }

}  // namespace detail


class ConsoleReporter : public IReporter {
public:
    std::string getName() const override { return "console"; }

    virtual void testStart() override {
        setlocale(LC_ALL, "en_US.utf8");
        std::cerr << "ZeroErr Unit Test" << std::endl;
    }

    virtual void testEnd(const TestContext& sum) override {
        std::cerr << "----------------------------------------------------------------"
                  << std::endl;
        std::cerr << "             " << FgGreen << "PASSED" << Reset << "   |   " << FgYellow
                  << "WARNING" << Reset << "   |   " << FgRed << "FAILED" << Reset << "   |   "
                  << Dim << "SKIPPED" << Reset << std::endl;
        std::cerr << "TEST CASE:   " << std::setw(6) << sum.passed << "       " << std::setw(7)
                  << sum.warning << "       " << std::setw(6) << sum.failed << "       "
                  << std::setw(7) << sum.skipped << std::endl;
        std::cerr << "ASSERTION:   " << std::setw(6) << sum.passed_as << "       " << std::setw(7)
                  << sum.warning_as << "       " << std::setw(6) << sum.failed_as << "       "
                  << std::setw(7) << sum.skipped_as << std::endl;
    }

    virtual void testCaseStart(const TestCase& tc, std::stringbuf&) override {
        std::cerr << "TEST CASE " << Dim << "[" << getFileName(tc.file) << ":" << tc.line << "] "
                  << Reset << FgCyan << tc.name << Reset << std::endl;
    }

    virtual void testCaseEnd(const TestCase&, std::stringbuf& sb, const TestContext&,
                             int type) override {
        if (!(ut.silent && type == 0)) std::cerr << insertIndentation(sb.str()) << std::endl;
    }

    virtual void subCaseStart(const TestCase& tc, std::stringbuf&) override {
        std::cerr << "SUB CASE " << Dim << "[" << getFileName(tc.file) << ":" << tc.line << "] "
                  << Reset << FgCyan << tc.name << Reset << std::endl;
    }

    virtual void subCaseEnd(const TestCase&, std::stringbuf& sb, const TestContext&,
                            int type) override {
        if (!(ut.silent && type == 0)) std::cerr << insertIndentation(sb.str()) << std::endl;
    }

    ConsoleReporter(UnitTest& ut) : IReporter(ut) {}
};


namespace detail {

// =================================================================================================
// The following code has been taken verbatim from Catch2/include/internal/catch_xmlwriter.h/cpp
// =================================================================================================
class XmlEncode {
public:
    enum ForWhat { ForTextNodes, ForAttributes };
    XmlEncode(std::string const& str, ForWhat forWhat = ForTextNodes);
    void                 encodeTo(std::ostream& os) const;
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

    XmlWriter(std::ostream& os = std::cerr);
    ~XmlWriter();

    XmlWriter(XmlWriter const&)            = delete;
    XmlWriter& operator=(XmlWriter const&) = delete;

    XmlWriter&    startElement(std::string const& name);
    ScopedElement scopedElement(std::string const& name);
    XmlWriter&    endElement();

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
                // Escape control characters in standard ascii, see:
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
}  // namespace detail

class XmlReporter : public IReporter {
public:
    detail::XmlWriter xml;

    struct TestCaseData {
        struct TestCase {
            std::string filename, name;
            unsigned    line;
            double      time;
            TestContext context;
            std::string output;
        };

        std::vector<TestCase> testcases;
        double                total_time;
    } tc_data;
    std::vector<const TestCase*> current;

    virtual std::string getName() const override { return "xml"; }

    // There are a list of events
    virtual void testStart() override { xml.writeDeclaration(); }

    virtual void testCaseStart(const TestCase& tc, std::stringbuf&) override {
        current.push_back(&tc);
    }

    virtual void testCaseEnd(const TestCase& tc, std::stringbuf& sb, const TestContext& ctx,
                             int) override {
        tc_data.testcases.push_back({tc.file, tc.name, tc.line, 0.0, ctx, sb.str()});
        current.pop_back();
    }

    virtual void subCaseStart(const TestCase& tc, std::stringbuf&) override {
        current.push_back(&tc);
    }

    virtual void subCaseEnd(const TestCase& tc, std::stringbuf& sb, const TestContext& ctx,
                            int) override {
        tc_data.testcases.push_back({tc.file, tc.name, tc.line, 0.0, ctx, sb.str()});
        current.pop_back();
    }

    virtual void testEnd(const TestContext& tc) override {
        xml.startElement("ZeroErr")
            .writeAttribute("binary", ut.binary)
            .writeAttribute("version", ZEROERR_VERSION_STR);
        xml.startElement("OverallResults")
            .writeAttribute("errors", tc.failed_as)
            .writeAttribute("failures", tc.failed)
            .writeAttribute("tests", tc.passed + tc.failed + tc.warning);
        xml.endElement();
        xml.startElement("TestSuite");
        for (const auto& testCase : tc_data.testcases) {
            xml.startElement("TestCase")
                .writeAttribute("name", testCase.name)
                .writeAttribute("filename", testCase.filename)
                .writeAttribute("line", testCase.line)
                .writeAttribute("skipped", "false");
            xml.scopedElement("Result")
                .writeAttribute("time", testCase.time)
                .writeAttribute("passed", testCase.context.passed)
                .writeAttribute("warnings", testCase.context.warning)
                .writeAttribute("failed", testCase.context.failed)
                .writeAttribute("skipped", testCase.context.skipped);
            xml.scopedElement("ResultAsserts")
                .writeAttribute("passed", testCase.context.passed_as)
                .writeAttribute("warnings", testCase.context.warning_as)
                .writeAttribute("failed", testCase.context.failed_as)
                .writeAttribute("skipped", testCase.context.skipped_as);
            xml.scopedElement("Output")
                .writeText(testCase.output);
            xml.endElement();
        }
        xml.endElement();
        xml.endElement();
    }

    XmlReporter(UnitTest& ut) : IReporter(ut), xml(std::cout) {}
};

IReporter* IReporter::create(const std::string& name, UnitTest& ut) {
    if (name == "console") return new ConsoleReporter(ut);
    if (name == "xml") return new XmlReporter(ut);
    return nullptr;
}


}  // namespace zeroerr


int main(int argc, const char** argv) {
    zeroerr::UnitTest().parseArgs(argc, argv).run();
    std::_Exit(0);
    return 0;
}
