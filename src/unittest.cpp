#include <fstream>
#include <chrono>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "zeroerr/unittest.h"
#include "zeroerr/assert.h"
#include "zeroerr/color.h"
#include "zeroerr/internal/fuzz_signal.h"
#include "zeroerr/internal/threadsafe.h"
#include "zeroerr/log.h"

#include <iomanip>
#include <iostream>
#include <memory>
#include <locale>
#include <ostream>
#include <sstream>
#include <regex>
#include <set>
#include <string>
#include <vector>

#ifdef ZEROERR_ENABLE_COVERAGE
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <cerrno>
#ifdef _WIN32
#include <process.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#endif

#ifndef ZEROERR_COVERAGE_PYTHON
#define ZEROERR_COVERAGE_PYTHON "python3"
#endif
#ifndef ZEROERR_COVERAGE_SCRIPT
#define ZEROERR_COVERAGE_SCRIPT ""
#endif
#ifndef ZEROERR_COVERAGE_SOURCE_ROOT
#define ZEROERR_COVERAGE_SOURCE_ROOT ""
#endif
#ifndef ZEROERR_COVERAGE_PROFDATA
#define ZEROERR_COVERAGE_PROFDATA "llvm-profdata"
#endif
#ifndef ZEROERR_COVERAGE_COV
#define ZEROERR_COVERAGE_COV "llvm-cov"
#endif

extern "C" {
void __llvm_profile_reset_counters(void);
uint64_t __llvm_profile_get_size_for_buffer(void);
int __llvm_profile_write_buffer(char* buffer);
}
#endif

namespace zeroerr {

#ifdef ZEROERR_ENABLE_COVERAGE
namespace {
int coverageCommand(const UnitTest& config) {
    const char* python_override = std::getenv("ZEROERR_COVERAGE_PYTHON");
    const char* script_override = std::getenv("ZEROERR_COVERAGE_SCRIPT");
    std::filesystem::path script = script_override ? script_override : ZEROERR_COVERAGE_SCRIPT;
    if (!script_override && (script.empty() || !std::filesystem::is_regular_file(script)))
        script = std::filesystem::absolute(config.binary.c_str()).parent_path() / "zeroerr-coverage.py";
    if (!std::filesystem::is_regular_file(script)) {
        std::cerr << "Coverage helper not found. Set ZEROERR_COVERAGE_SCRIPT to coverage-report.py, "
                     "or place zeroerr-coverage.py beside the executable.\n";
        return 2;
    }
    std::vector<std::string> arguments = {
        python_override ? python_override : ZEROERR_COVERAGE_PYTHON,
        script.string(), "--run", config.binary,
        "--llvm-profdata", ZEROERR_COVERAGE_PROFDATA,
        "--llvm-cov", ZEROERR_COVERAGE_COV,
    };
    if (std::string(ZEROERR_COVERAGE_SOURCE_ROOT).size()) {
        arguments.push_back("--source-root");
        arguments.push_back(ZEROERR_COVERAGE_SOURCE_ROOT);
    }
    std::vector<std::string> test_arguments;
    for (const auto& stored_arg : config.coverage_arguments) {
        const std::string arg(stored_arg.c_str());
        if (arg == "--coverage" || arg == "-c") continue;
        const std::pair<const char*, const char*> options[] = {
            {"--coverage-output=", "--output-dir"},
            {"--coverage-include=", "--include"},
            {"--coverage-exclude=", "--exclude"},
        };
        bool consumed = false;
        for (const auto& option : options) {
            const std::string prefix(option.first);
            if (arg.compare(0, prefix.size(), prefix) == 0) {
                arguments.push_back(option.second);
                arguments.push_back(arg.substr(prefix.size()));
                consumed = true;
                break;
            }
        }
        if (!consumed) test_arguments.push_back(arg);
    }
    arguments.push_back("--");
    arguments.insert(arguments.end(), test_arguments.begin(), test_arguments.end());
    // Pass an argv array, never shell-interpolate paths, regexes or test names.
#ifdef _WIN32
    // The Windows spawn API joins argv into a command line without quoting it.
    // Encode each argument using the receiving CRT's backslash/quote rules.
    const std::string program = arguments[0];
    for (auto& arg : arguments) {
        std::string quoted = "\"";
        size_t slashes = 0;
        for (char c : arg) {
            if (c == '\\') { ++slashes; continue; }
            quoted.append(c == '"' ? slashes * 2 + 1 : slashes, '\\');
            quoted += c;
            slashes = 0;
        }
        quoted.append(slashes * 2, '\\');
        arg = quoted + '"';
    }
#endif
    std::vector<char*> argv;
    for (auto& arg : arguments) argv.push_back(const_cast<char*>(arg.c_str()));
    argv.push_back(nullptr);
#ifdef _WIN32
    const auto status = _spawnvp(_P_WAIT, program.c_str(), argv.data());
    if (status >= 0) return static_cast<int>(status);
#else
    const pid_t pid = fork();
    if (pid == 0) {
        execvp(argv[0], argv.data());
        std::cerr << "Could not start Python; set ZEROERR_COVERAGE_PYTHON to a Python 3 executable.\n";
        _exit(2);
    }
    if (pid > 0) {
        int status = 0;
        pid_t waited;
        do { waited = waitpid(pid, &status, 0); } while (waited < 0 && errno == EINTR);
        if (waited == pid && WIFEXITED(status)) return WEXITSTATUS(status);
    }
#endif
    std::cerr << "Could not run coverage helper; check Python and ZEROERR_COVERAGE_PYTHON.\n";
    return 2;
}

std::string coverageJsonString(const std::string& value) {
    std::ostringstream out;
    out << '"';
    for (unsigned char c : value) {
        if (c == '"' || c == '\\') out << '\\' << c;
        else if (c < 0x20) {
            out << "\\u00" << "0123456789abcdef"[c >> 4] << "0123456789abcdef"[c & 15];
        } else out << c;
    }
    out << '"';
    return out.str();
}

// Buffer snapshots avoid filename merging and the runtime's exit-time writer
// overwriting the last test. Subcases deliberately share their parent's counters.
class CoverageSession {
    std::filesystem::path directory;
    std::ofstream manifest;
    unsigned sequence = 0;
public:
    explicit CoverageSession(const std::string& output) : directory(output) {
        if (output.empty()) return;
        if (!std::filesystem::create_directories(directory))
            throw std::runtime_error("coverage output directory already exists: " + output);
        manifest.exceptions(std::ios::badbit | std::ios::failbit);
        manifest.imbue(std::locale::classic());
        manifest.open(directory / "manifest.jsonl");
        manifest << "{\"schema\":1,\"granularity\":\"test_case\"}\n";
    }
    void begin() {
        if (manifest.is_open()) __llvm_profile_reset_counters();
    }
    void end(const TestCase& tc, const TestContext& ctx, int result) {
        if (!manifest.is_open()) return;
        std::vector<char> buffer(static_cast<size_t>(__llvm_profile_get_size_for_buffer()));
        if (__llvm_profile_write_buffer(buffer.data()) != 0)
            throw std::runtime_error("could not snapshot Clang coverage counters");
        const std::string profile = "test-" + std::to_string(sequence++) + ".profraw";
        std::ofstream raw;
        raw.exceptions(std::ios::badbit | std::ios::failbit);
        raw.open(directory / profile, std::ios::binary);
        raw.write(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        raw.close();
        manifest << "{\"name\":" << coverageJsonString(tc.name)
                 << ",\"file\":" << coverageJsonString(tc.file)
                 << ",\"line\":" << tc.line
                 << ",\"seconds\":" << std::setprecision(17) << ctx.duration.count()
                 << ",\"result\":" << result
                 << ",\"profile\":" << coverageJsonString(profile) << "}\n";
        manifest.flush();
    }
    void finish() {
        if (manifest.is_open()) {
            manifest << "{\"complete\":true}\n";
            manifest.close();
        }
    }
};
} // namespace
#endif

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

static inline zeroerr::String getFileName(zeroerr::String file) {
    zeroerr::String fileName(file);
    auto        p = fileName.find_last_of('/');
    if (p == zeroerr::String::npos) p = fileName.find_last_of('\\');
    if (p != zeroerr::String::npos) fileName = fileName.substr(p + 1);
    return fileName;
}

SubCase::SubCase(zeroerr::String name, zeroerr::String file, unsigned line, TestContext* context,
                 std::vector<Decorator*> decorators)
    : TestCase(name, file, line, decorators), context(context) {}

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
        // A helper may throw without having received this subcase's context.
        if (local.failed_as == 0) local.failed_as = 1;
    } catch (const FuzzFinishedException&) {
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

struct Filters {
    std::vector<std::regex> name, name_exclude;
    std::vector<std::regex> file, file_exclude;
};

static void printUsage() {
    std::cout << "Usage: <test-binary> [options]\n"
              << "\n"
              << "Options:\n"
              << "  -h, --help                Show this help message\n"
              << "  -v, --verbose             Print all test results\n"
              << "  -q, --quiet               Suppress output for passing tests\n"
              << "  -l, --list-test-cases     List registered test cases\n"
              << "  -b, --bench               Also run benchmark tests\n"
              << "  -f, --fuzz                Also run fuzz tests\n"
              << "      --list-format=FORMAT  Listing format: \"console\" (default) or \"plain\"\n"
              << "      --testcase=REGEX      Run only test cases whose name matches REGEX\n"
              << "      --testcase-exclude=REGEX\n"
              << "                            Skip test cases whose name matches REGEX\n"
              << "      --file=REGEX          Run only test cases in files matching REGEX\n"
              << "      --file-exclude=REGEX  Skip test cases in files matching REGEX\n"
              << "      --reporters=NAME      Reporter to use: \"console\" or \"xml\"\n"
              << "  -x                        Shortcut for --reporters=xml\n"
              << "      --no-color            Disable colored output\n"
              << "      --log-to-report       Include log messages in the report\n"
              << "  -c, --coverage           Collect coverage and generate a report (Clang build)\n"
              << "      --coverage-output=DIR Write profiles to a new directory\n"
              << "      --coverage-include=REGEX Select source paths for the coverage report\n"
              << "      --coverage-exclude=REGEX Exclude source paths from the coverage report\n";
}

UnitTest& UnitTest::parseArgs(int argc, const char** argv) {
    delete filters;
    filters = new Filters();

    auto add_regex = [&](std::vector<std::regex>& vec, const zeroerr::String& pattern,
                         const char* option) {
        try {
            vec.push_back(std::regex(pattern.c_str()));
        } catch (const std::regex_error& e) {
            std::cerr << "Invalid regex for " << option << ": " << e.what() << std::endl;
            this->invalid_args = true;
        }
    };

    auto convert_to_vec = [=]() {
        std::vector<zeroerr::String> result;
        for (int i = 1; i < argc; i++) {
            result.emplace_back(argv[i]);
        }
        return result;
    };

    auto parse_char = [&](char arg) {
        if (arg == 'c') {
            this->run_coverage = true;
            return true;
        }
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
        if (arg == 'h') {
            this->show_help = true;
            return true;
        }
        return false;
    };

    auto parse_token = [&](zeroerr::String arg) {
        if (arg == "coverage") {
            this->run_coverage = true;
            return true;
        }
        if (arg.substr(0, 16) == "coverage-output=") {
            this->coverage_output = arg.substr(16);
            if (coverage_output.empty()) {
                std::cerr << "coverage-output needs a directory\n";
                invalid_args = true;
            }
            return true;
        }
        if (arg.find("coverage-include=") == 0 || arg.find("coverage-exclude=") == 0) return true;
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
            return true;
        }
        if (arg == "fuzz") {
            this->run_fuzz = true;
            return true;
        }
        if (arg == "list-test-cases") {
            this->list_test_cases = true;
            return true;
        }
        if (arg == "no-color") {
            this->no_color = true;
#if !defined(ZEROERR_ALWAYS_COLORFUL) && !defined(ZEROERR_DISABLE_COLORFUL)
            disableColorOutput();
#endif
            return true;
        }
        if (arg == "log-to-report") {
            this->log_to_report = true;
            return true;
        }
        if (arg == "help") {
            this->show_help = true;
            return true;
        }
        if (arg.find("list-format=") == 0) {
            this->list_format = arg.substr(12);
            return true;
        }
        if (arg.find("reporters=") == 0) {
            this->reporter_name = arg.substr(10);
            return true;
        }
        // Keep the longest prefixes first. The '='-suffixed comparisons are
        // already unambiguous ("testcase=" != "testcase-exclude="), but a bare
        // prefix check such as `testcase` would swallow `testcase-exclude=...`
        // and parse it as an include filter.
        if (arg.find("testcase-exclude=") == 0) {
            add_regex(filters->name_exclude, arg.substr(17), "--testcase-exclude=");
            return true;
        }
        if (arg.find("testcase=") == 0) {
            add_regex(filters->name, arg.substr(9), "--testcase=");
            return true;
        }
        if (arg.find("file-exclude=") == 0) {
            add_regex(filters->file_exclude, arg.substr(13), "--file-exclude=");
            return true;
        }
        if (arg.find("file=") == 0) {
            add_regex(filters->file, arg.substr(5), "--file=");
            return true;
        }
        std::cerr << "Unknown option: --" << arg << std::endl;
        this->invalid_args = true;
        return false;
    };

    auto parse_pos = [&](const std::vector<zeroerr::String>& args, size_t pos) {
        if (args[pos].size() == 2 && args[pos][0] == '-') {
            return parse_char(args[pos][1]);
        }
        if (args[pos].size() > 2 && args[pos][0] == '-' && args[pos][1] == '-') {
            return parse_token(args[pos].substr(2));
        }
        return false;
    };

    auto args = convert_to_vec();
    coverage_arguments = args;
    for (size_t i = 0; i < args.size(); ++i) parse_pos(args, i);

    binary = argv[0];
    return *this;
}


static zeroerr::String insertIndentation(zeroerr::String str) {
    std::stringstream result;
    std::stringstream ss(str);

    zeroerr::String line;
    while (zeroerr::getline(ss, line)) {
        result << line << std::endl << "    ";
    }

    return result.str();
}

bool UnitTest::run_filter(const TestCase& tc) {
    if (filters == nullptr) return true;
    for (auto& r : filters->name)
        if (!std::regex_match(tc.name.c_str(), r)) return false;
    for (auto& r : filters->name_exclude)
        if (std::regex_match(tc.name.c_str(), r)) return false;
    for (auto& r : filters->file)
        if (!std::regex_match(tc.file.c_str(), r)) return false;
    for (auto& r : filters->file_exclude)
        if (std::regex_match(tc.file.c_str(), r)) return false;
    return true;
}

static bool runOnExecution(const TestCase& tc) {
    for (auto& decorator : tc.decorators) {
        if (decorator->onExecution(tc)) return true;
    }
    return false;
}

static bool runOnFinish(const TestCase& tc, TestContext& ctx) {
    bool contain_changes = false;
    for (auto& decorator : tc.decorators) {
        if (decorator->onFinish(tc, ctx)) {
            contain_changes = true;
        }
    }
    return contain_changes;
}

int UnitTest::run() {
    if (show_help) {
        printUsage();
        return 0;
    }
    if (invalid_args) {
        return 1;
    }

    unsigned types = TestType::test_case;
    if (run_bench) types |= TestType::bench;
    if (run_fuzz) types |= TestType::fuzz_test;
    std::set<TestCase> test_cases = detail::getRegisteredTests(types);

    // Machine-readable listing: one "<name>\t<basename>:<line>" entry per test
    // case on stdout. This is the format consumed by CTest discovery scripts
    // (e.g. EVEngine's cmake/ZeroErrDiscoverTestsImpl.cmake).
    if (list_test_cases && list_format == "plain") {
        for (auto& tc : test_cases) {
            if (!run_filter(tc)) continue;
            if (runOnExecution(tc)) continue;
            std::cout << tc.name << '\t' << getFileName(tc.file) << ':' << tc.line << '\n';
        }
        return 0;
    }

    if (run_coverage && !list_test_cases) {
#ifdef ZEROERR_ENABLE_COVERAGE
        try { return coverageCommand(*this); }
        catch (const std::exception& e) {
            std::cerr << "Coverage: " << e.what() << '\n';
            return 2;
        }
#else
        std::cerr << "Configure with Clang and -DZEROERR_ENABLE_COVERAGE=ON, then rerun --coverage.\n";
        return 2;
#endif
    }
#ifndef ZEROERR_ENABLE_COVERAGE
    if (!coverage_output.empty()) {
        std::cerr << "Coverage requires a build with ZEROERR_ENABLE_COVERAGE and Clang instrumentation.\n";
        return 2;
    }
#else
    // Listing must never reset counters or create output files.
    std::unique_ptr<CoverageSession> coverage;
    try {
        coverage.reset(new CoverageSession(list_test_cases ? "" : coverage_output.c_str()));
    } catch (const std::exception& e) {
        std::cerr << "Coverage: " << e.what() << '\n';
        return 2;
    }
    bool coverage_failed = false;
#endif
    IReporter* reporter = IReporter::create(reporter_name, *this);
    if (!reporter) reporter = IReporter::create("console", *this);

    TestContext context(*reporter), sum(*reporter);
    reporter->testStart();
    std::stringbuf new_buf;

    for (auto& tc : test_cases) {
        if (!run_filter(tc)) continue;
        if (runOnExecution(tc)) {
            sum.skipped += 1;
            continue;
        }
        reporter->testCaseStart(tc, new_buf);
        if (!list_test_cases) {
            std::streambuf* orig_buf = std::cerr.rdbuf();
            std::cerr.rdbuf(&new_buf);
            std::cerr << std::endl;
#ifdef ZEROERR_ENABLE_COVERAGE
            coverage->begin();
#endif
            auto start = std::chrono::high_resolution_clock::now();
            try {
                tc.func(&context);  // run the test case
            } catch (const AssertionData&) {
                // Context-free helper assertions must not disappear at the catch.
                if (context.failed_as == 0) context.failed_as = 1;
            } catch (const FuzzFinishedException&) {
            } catch (const std::exception& e) {
                std::cerr << e.what() << std::endl;
                if (context.failed_as == 0) {
                    context.failed_as = 1;
                }
            }
            auto end         = std::chrono::high_resolution_clock::now();
            context.duration.seconds = std::chrono::duration<double>(end - start).count();
            std::cerr.rdbuf(orig_buf);
        }
        // Decorators may rewrite pass/fail before results are accumulated.
        runOnFinish(tc, context);
        int type = sum.add(context);
#ifdef ZEROERR_ENABLE_COVERAGE
        if (!list_test_cases) {
            try {
                coverage->end(tc, context, type);
            } catch (const std::exception& e) {
                std::cerr << "Coverage: " << e.what() << '\n';
                coverage_failed = true;
            }
        }
#endif
        reporter->testCaseEnd(tc, new_buf, context, type);
        context.reset();
        new_buf.str("");
    }
    reporter->testEnd(sum);
    delete reporter;
#ifdef ZEROERR_ENABLE_COVERAGE
    try {
        if (!coverage_failed) coverage->finish();
    } catch (const std::exception& e) {
        std::cerr << "Coverage: " << e.what() << '\n';
        coverage_failed = true;
    }
    if (coverage_failed) return 2;
#endif
#ifdef ZEROERR_CHECK_AS_ERROR
    return (sum.failed > 0 || sum.failed_as > 0 || sum.warning_as > 0) ? 1 : 0;
#else
    return (sum.failed > 0 || sum.failed_as > 0) ? 1 : 0;
#endif
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
    throw std::runtime_error("Invalid test type");
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

regTest::regTest(const TestCase& tc, TestType type) {
    for (auto& decorator : tc.decorators) {
        if (decorator->onStartup(tc)) return;
    }
    getTestSet(type).insert(tc);
}

static std::set<IReporter*>& getRegisteredReporters() {
    static std::set<IReporter*> data;
    return data;
}

regReporter::regReporter(IReporter* reporter) { getRegisteredReporters().insert(reporter); }

}  // namespace detail


class ConsoleReporter : public IReporter {
public:
    zeroerr::String getName() const override { return "console"; }

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
                  << Reset << FgCyan << tc.name << Reset;
    }

    virtual void testCaseEnd(const TestCase&, std::stringbuf& sb, const TestContext&,
                             int type) override {
        if (!(ut.silent && type == 0)) std::cerr << "  " << (type == 0 ? "✅" : type == 1 ? "⚠️" : "❌")
        << std::endl << insertIndentation(sb.str()) << std::endl;
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
    XmlEncode(const zeroerr::String& str, ForWhat forWhat = ForTextNodes);
    void                 encodeTo(std::ostream& os) const;
    friend std::ostream& operator<<(std::ostream& os, const XmlEncode& xmlEncode);

private:
    zeroerr::String m_str;
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

        ScopedElement& writeText(const zeroerr::String& text, bool indent = true, bool new_line = true);

        template <typename T>
        ScopedElement& writeAttribute(const zeroerr::String& name, const T& attribute) {
            m_writer->writeAttribute(name, attribute);
            return *this;
        }

    private:
        mutable XmlWriter* m_writer = nullptr;
    };

    XmlWriter(std::ostream& os = std::cerr);
    ~XmlWriter();

    XmlWriter(const XmlWriter&)            = delete;
    XmlWriter& operator=(const XmlWriter&) = delete;

    XmlWriter&    startElement(const zeroerr::String& name);
    ScopedElement scopedElement(const zeroerr::String& name);
    XmlWriter&    endElement();

    XmlWriter& writeAttribute(const zeroerr::String& name, const zeroerr::String& attribute);
    XmlWriter& writeAttribute(const zeroerr::String& name, const char* attribute);
    XmlWriter& writeAttribute(const zeroerr::String& name, bool attribute);

    template <typename T>
    XmlWriter& writeAttribute(const zeroerr::String& name, const T& attribute) {
        std::stringstream rss;
        rss << attribute;
        return writeAttribute(name, zeroerr::String(rss.str()));
    }

    XmlWriter& writeText(const zeroerr::String& text, bool indent = true, bool new_line = true);

    void ensureTagClosed(bool new_line = true);
    void writeDeclaration();

private:
    void newlineIfNecessary();

    bool                     m_tagIsOpen    = false;
    bool                     m_needsNewline = false;
    bool                     m_needsIndent  = false;
    std::vector<zeroerr::String> m_tags;
    zeroerr::String              m_indent;
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

XmlEncode::XmlEncode(const zeroerr::String& str, ForWhat forWhat) : m_str(str), m_forWhat(forWhat) {}

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

std::ostream& operator<<(std::ostream& os, const XmlEncode& xmlEncode) {
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

XmlWriter::ScopedElement& XmlWriter::ScopedElement::writeText(const zeroerr::String& text, bool indent,
                                                              bool new_line) {
    m_writer->writeText(text, indent, new_line);
    return *this;
}

XmlWriter::XmlWriter(std::ostream& os) : m_os(os) {}

XmlWriter::~XmlWriter() {
    while (!m_tags.empty()) endElement();
}

XmlWriter& XmlWriter::startElement(const zeroerr::String& name) {
    ensureTagClosed();
    newlineIfNecessary();
    m_os << m_indent << '<' << name;
    m_tags.push_back(name);
    m_indent += "  ";
    m_tagIsOpen = true;
    return *this;
}

XmlWriter::ScopedElement XmlWriter::scopedElement(const zeroerr::String& name) {
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
        if (m_needsIndent)
            m_os << m_indent;
        else
            m_needsIndent = true;
        m_os << "</" << m_tags.back() << ">";
    }
    m_os << std::endl;
    m_tags.pop_back();
    return *this;
}

XmlWriter& XmlWriter::writeAttribute(const zeroerr::String& name, const zeroerr::String& attribute) {
    if (!name.empty() && !attribute.empty())
        m_os << ' ' << name << "=\"" << XmlEncode(attribute, XmlEncode::ForAttributes) << '"';
    return *this;
}

XmlWriter& XmlWriter::writeAttribute(const zeroerr::String& name, const char* attribute) {
    if (!name.empty() && attribute && attribute[0] != '\0')
        m_os << ' ' << name << "=\"" << XmlEncode(attribute, XmlEncode::ForAttributes) << '"';
    return *this;
}

XmlWriter& XmlWriter::writeAttribute(const zeroerr::String& name, bool attribute) {
    m_os << ' ' << name << "=\"" << (attribute ? "true" : "false") << '"';
    return *this;
}

XmlWriter& XmlWriter::writeText(const zeroerr::String& text, bool indent, bool new_line) {
    if (!text.empty()) {
        bool tagWasOpen = m_tagIsOpen;
        ensureTagClosed(new_line);
        if (tagWasOpen && indent) m_os << m_indent;
        m_os << XmlEncode(text);
        m_needsNewline = new_line;
        m_needsIndent  = new_line;
    }
    return *this;
}

void XmlWriter::ensureTagClosed(bool new_line) {
    if (m_tagIsOpen) {
        m_os << ">";
        if (new_line) m_os << std::endl;
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

    struct TestCaseTemp {
        const TestCase* tc;
    };

    std::vector<TestCaseTemp> current;

    virtual zeroerr::String getName() const override { return "xml"; }

    // There are a list of events
    virtual void testStart() override {
        xml.writeDeclaration();
        xml.startElement("ZeroErr")
            .writeAttribute("binary", ut.binary)
            .writeAttribute("version", ZEROERR_VERSION_STR);
        xml.startElement("TestSuite");
    }

    virtual void testCaseStart(const TestCase& tc, std::stringbuf&) override {
        current.push_back({&tc});
        xml.startElement("TestCase")
            .writeAttribute("name", tc.name)
            .writeAttribute("filename", tc.file)
            .writeAttribute("line", tc.line)
            .writeAttribute("skipped", "false");
        if (ut.log_to_report) suspendLog();
    }

    virtual void testCaseEnd(ZEROERR_UNUSED(const TestCase&), std::stringbuf& sb,
                             const TestContext& ctx, int) override {
        current.pop_back();
        xml.scopedElement("Result")
            .writeAttribute("time", 0)
            .writeAttribute("passed", ctx.passed)
            .writeAttribute("warnings", ctx.warning)
            .writeAttribute("failed", ctx.failed)
            .writeAttribute("skipped", ctx.skipped);
        xml.scopedElement("ResultAsserts")
            .writeAttribute("passed", ctx.passed_as)
            .writeAttribute("warnings", ctx.warning_as)
            .writeAttribute("failed", ctx.failed_as)
            .writeAttribute("skipped", ctx.skipped_as);
        xml.scopedElement("Output").writeText(sb.str());

        if (ut.log_to_report) {
            xml.startElement("Log");
            LogIterator begin = LogStream::getDefault().begin();
            LogIterator end   = LogStream::getDefault().end();
            for (auto p = begin; p != end; ++p) {
                xml.startElement("LogEntry")
                    .writeAttribute("function", p->info->function)
                    .writeAttribute("line", p->info->line)
                    .writeAttribute("message", p->info->message)
                    .writeAttribute("category", p->info->category)
                    .writeAttribute("severity", p->info->severity);
                for (auto pair : p->getData()) {
                    xml.scopedElement(pair.first).writeText(pair.second, false, false);
                }
                xml.endElement();
            }
            xml.endElement();
            resumeLog();
        }
        xml.endElement();
    }

    virtual void subCaseStart(const TestCase& tc, std::stringbuf& sb) override {
        testCaseStart(tc, sb);
    }

    virtual void subCaseEnd(const TestCase& tc, std::stringbuf& sb, const TestContext& ctx,
                            int) override {
        testCaseEnd(tc, sb, ctx, 0);
    }

    virtual void testEnd(const TestContext& tc) override {
        xml.endElement();

        xml.startElement("OverallResults")
            .writeAttribute("errors", tc.failed_as)
            .writeAttribute("failures", tc.failed)
            .writeAttribute("tests", tc.passed + tc.failed + tc.warning);
        xml.endElement();

        xml.endElement();
    }

    XmlReporter(UnitTest& ut) : IReporter(ut), xml(std::cout) {}
};

IReporter* IReporter::create(const zeroerr::String& name, UnitTest& ut) {
    if (name == "console") return new ConsoleReporter(ut);
    if (name == "xml") return new XmlReporter(ut);
    return nullptr;
}


class SkipDecorator : public Decorator {
    bool onExecution(const TestCase&) override { return true; }
};

Decorator* skip(bool isSkip) {
    static SkipDecorator skip_dec;
    if (isSkip) return &skip_dec;
    return nullptr;
}

class TimeoutDecorator : public Decorator {
    float timeout;

public:
    TimeoutDecorator() : timeout(0) {}
    TimeoutDecorator(float timeout) : timeout(timeout) {}

    bool onFinish(const TestCase&, TestContext& ctx) override {
        if (ctx.duration > std::chrono::duration<double>(timeout)) {
            std::cerr << FgRed <<  "Timeout: " << Reset << ctx.duration.count() << "s > " << timeout << "s" << std::endl;
            return true;
        }
        return false;
    }
};

Decorator* timeout(float timeout) {
    static std::map<float, TimeoutDecorator> timeout_dec;
    if (timeout_dec.find(timeout) == timeout_dec.end()) {
        timeout_dec[timeout] = TimeoutDecorator(timeout);
    }
    return &timeout_dec[timeout];
}

class FailureDecorator : public Decorator {
public:
    enum FailureType { may_fail, should_fail };
    FailureDecorator(FailureType type) : type(type) {}

    bool onFinish(const TestCase&, TestContext& ctx) override {
        if (type == FailureType::may_fail) {
            // Treat failures as warnings so the suite can continue cleanly.
            ctx.warning_as += ctx.failed_as;
            ctx.failed_as = 0;
            ctx.warning += ctx.failed;
            ctx.failed = 0;
            return true;
        }
        if (type == FailureType::should_fail) {
            if (ctx.failed_as > 0 || ctx.failed > 0) {
                ctx.passed_as += ctx.failed_as;
                ctx.failed_as = 0;
                ctx.passed += ctx.failed;
                ctx.failed = 0;
            } else {
                ctx.failed_as = ctx.passed_as > 0 ? ctx.passed_as : 1;
                ctx.passed_as = 0;
                ctx.failed = 1;
                ctx.passed = 0;
            }
            return true;
        }
        return false;
    }

private:
    FailureType type;
};


Decorator* may_fail(bool isMayFail) {
    static FailureDecorator may_fail_dec(FailureDecorator::may_fail);
    if (isMayFail) return &may_fail_dec;
    return nullptr;
}

Decorator* should_fail(bool isShouldFail) {
    static FailureDecorator should_fail_dec(FailureDecorator::should_fail);
    if (isShouldFail) return &should_fail_dec;
    return nullptr;
}


}  // namespace zeroerr


#ifndef ZEROERR_NO_MAIN
int main(int argc, const char** argv) {
    return zeroerr::UnitTest().parseArgs(argc, argv).run();
}
#endif
