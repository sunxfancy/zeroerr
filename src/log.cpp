#include <stdexcept>
#include <sstream>
#include <iostream>
#include <chrono>
#include "zeroerr/log.h"
#include "zeroerr/internal/threadsafe.h"

#include <cstdio>
#include <cstddef>
#include <ctime>
#include <iomanip>
#include <unordered_set>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#endif

const char* ZEROERR_LOG_CATEGORY = "default";


namespace zeroerr {

int _ZEROERR_G_VERBOSE = 0;

thread_local std::vector<IContextScope*> _ZEROERR_G_CONTEXT_SCOPE_VECTOR;

static zeroerr::String       DefaultLogCallback(const LogMessage& msg, bool colorful);
static LogCustomCallback log_custom_callback = DefaultLogCallback;
void setLogCustomCallback(LogCustomCallback callback) { log_custom_callback = callback; }


LogInfo::LogInfo(const char* filename, const char* function, const char* message,
                 const char* category, unsigned line, unsigned size, LogSeverity severity)
    : filename(filename),
      function(function),
      message(message),
      category(category),
      line(line),
      size(size),
      severity(severity),
      names() {
    for (const char* p = message; *p; p++)
        if (*p == '{') {
            const char* q = p + 1;
            while (*q && *q != '}') q++;
            if (*q == '}') {
                zeroerr::String N(p + 1, (size_t)(q - p - 1));
                names[N] = static_cast<int>(names.size());
                p        = q;
            }
        }
}


constexpr size_t LogStreamMaxSize = 64 * 1024 - 16;

struct DataBlock {
    ZEROERR_ATOMIC(size_t) size;
    DataBlock* next = nullptr;
    alignas(std::max_align_t) char data[LogStreamMaxSize];

    DataBlock() : size(0) {}

    ~DataBlock() { clear(); }
    void clear() {
        size_t offset = 0;
        while (offset < ZEROERR_LOAD(size)) {
            auto* message = reinterpret_cast<LogMessage*>(data + offset);
            offset += message->info->size;
            message->~LogMessage();
        }
        size = 0;
    }

    LogMessage* begin() { return (LogMessage*)data; }
    LogMessage* end() { return (LogMessage*)&data[size]; }
};

LogStream::LogStream() {
    first = m_last = new DataBlock();
#ifndef ZEROERR_NO_THREAD_SAFE
    mutex = new detail::Mutex();
#endif
    setStderrLogger();
}

LogStream::~LogStream() {
    while (first) {
        DataBlock* next = first->next;
        logger->flush(first);
        delete first;
        first = next;
    }
    if (logger) delete logger;
#ifndef ZEROERR_NO_THREAD_SAFE
    delete mutex;
#endif
}

void* LogStream::alloc_block(unsigned size) {
    if (size > LogStreamMaxSize) {
        throw std::runtime_error("LogStream::push: size > LogStreamMaxSize");
    }
    auto* last = ZEROERR_LOAD(m_last);
    if (last->size + size > LogStreamMaxSize) {
        if (flush_mode == FLUSH_WHEN_FULL) {
            logger->flush(last);
            last->clear();
        } else {
            last->next = new DataBlock();
            last       = last->next;
            m_last     = last;
        }
    }
    void* p = last->data + last->size;
    return p;
}

void LogStream::commit_block(DataBlock* block, unsigned size) {
    block->size += size;
}

LogIterator LogStream::current(zeroerr::String message, zeroerr::String function_name, int line) {
    LogIterator iter(*this, message, function_name, line);
    DataBlock*  last = ZEROERR_LOAD(m_last);
    iter.p           = last;
    iter.q           = reinterpret_cast<LogMessage*>(&(last->data[ZEROERR_LOAD(last->size)]));
    return iter;
}

void LogStream::flush() {
    ZEROERR_LOCK(*mutex);
    flush_unlocked();
}

void LogStream::flush_unlocked() {
    DataBlock* last = ZEROERR_LOAD(m_last);
    for (DataBlock* p = first; p != last;) {
        DataBlock* next = p->next;
        logger->flush(p);
        delete p;
        p = next;
    }
    logger->flush(last);
    last->clear();
    first      = last;
}

static LogMessage* moveBytes(LogMessage* p, unsigned size) {
    char* src = (char*)p;
    char* dst = src + size;
    return (LogMessage*)dst;
}

void* LogStream::getRawLog(zeroerr::String func, unsigned line, zeroerr::String name) {
    for (DataBlock* p = first; p; p = p->next)
        for (auto q = p->begin(); q < p->end(); q = moveBytes(q, q->info->size))
            if (line == q->info->line && func == q->info->function) return q->getRawLog(name);
    return nullptr;
}

static bool startWith(const zeroerr::String& str, const zeroerr::String& prefix) {
    return str.rfind(prefix, 0) == 0;
}

void* LogStream::getRawLog(zeroerr::String func, zeroerr::String msg, zeroerr::String name) {
    for (DataBlock* p = first; p; p = p->next)
        for (auto q = p->begin(); q < p->end(); q = moveBytes(q, q->info->size))
            if (startWith(q->info->message, msg) && func == q->info->function)
                return q->getRawLog(name);
    return nullptr;
}

LogIterator::LogIterator(LogStream& stream, zeroerr::String message, zeroerr::String function_name,
                         int line)
    : p(stream.first),
      q(stream.first->begin()),
      function_name_filter(function_name),
      message_filter(message),
      line_filter(line) {
    while (!check_filter() && p) next();
}

void LogIterator::check_at_safe_pos() {
    if (static_cast<size_t>((char*)q - p->data) >= ZEROERR_LOAD(p->size)) {
        p = p->next;
        q = reinterpret_cast<LogMessage*>(p->data);
    }
}

void LogIterator::next() {
    if (q < p->end()) {
        q = moveBytes(q, q->info->size);
        if (q >= p->end()) next();
    } else {
        p = p->next;
        if (p)
            q = p->begin();
        else
            q = nullptr;
    }
}

LogIterator& LogIterator::operator++() {
    do {
        next();
    } while (p && !check_filter());
    return *this;
}

bool LogIterator::check_filter() {
    if (!message_filter.empty() && startWith(q->info->message, message_filter)) return false;
    if (!function_name_filter.empty() && q->info->function != function_name_filter) return false;
    if (line_filter != -1 && static_cast<int>(q->info->line) != line_filter) return false;
    return true;
}

static FILE* openLogFile(const zeroerr::String& name) {
#ifdef _MSC_VER
    FILE* file = nullptr;
    fopen_s(&file, name.c_str(), "w");
    return file;
#else
    return std::fopen(name.c_str(), "w");
#endif
}

static std::tm logLocalTime(std::time_t time) {
    std::tm result{};
#ifdef _MSC_VER
    localtime_s(&result, &time);
#elif defined(__unix__) || defined(__APPLE__)
    localtime_r(&time, &result);
#else
    if (const std::tm* value = std::localtime(&time)) result = *value;
#endif
    return result;
}

class FileLogger : public Logger {
public:
    FileLogger(zeroerr::String name) { file = openLogFile(name); }
    ~FileLogger() {
        if (file) fclose(file);
    }
    void flush(DataBlock* msg) override {
        if (file) {
            zeroerr::String batch;
            for (auto p = msg->begin(); p < msg->end(); p = moveBytes(p, p->info->size)) {
                batch += log_custom_callback(*p, false);
            }
            fwrite(batch.c_str(), batch.size(), 1, file);
            fflush(file);
        }
    }

protected:
    FILE* file;
};


#ifdef _WIN32
static char split = '\\';
#else
static char split = '/';
#endif

static void make_dir(zeroerr::String path) {
    size_t pos = 0;
    do {
        pos             = path.find_first_of(split, pos + 1);
        zeroerr::String sub = path.substr(0, pos);
#ifdef _WIN32
        CreateDirectory(sub.c_str(), NULL);
#else
        struct stat st;
        if (stat(sub.c_str(), &st) == -1) {
            mkdir(sub.c_str(), 0755);
        }
#endif
    } while (pos != zeroerr::String::npos);
}

struct FileCache {
    std::map<zeroerr::String, FILE*> files;
    ~FileCache() {
        for (auto& p : files) {
            fclose(p.second);
        }
    }

    FILE* get(const zeroerr::String& name) {
        auto it = files.find(name);
        if (it != files.end()) return it->second;
        auto        p    = name.find_last_of(split);
        zeroerr::String path = name.substr(0, p);
        make_dir(path.c_str());

        FILE* file = openLogFile(name);
        if (file) {
            files[name] = file;
            return file;
        }
        return nullptr;
    }
};


class DirectoryLogger : public Logger {
public:
    DirectoryLogger(zeroerr::String path, LogStream::DirMode dir_mode[3]) : dirpath(path) {
        make_dir(path.c_str());
        for (int i = 0; i < 3; i++) {
            this->dir_mode[i] = dir_mode[i];
        }
    }
    ~DirectoryLogger() {}

    void flush(DataBlock* msg) override {
        FileCache cache;
        for (auto p = msg->begin(); p < msg->end(); p = moveBytes(p, p->info->size)) {
            auto ss = log_custom_callback(*p, false);

            std::stringstream path;
            path << dirpath;
            if (dirpath.back() != split) path << split;

            int last = 0;
            for (int i = 0; i < 3; i++) {
                if (last != 0 && dir_mode[i] != 0) path << split;
                switch (dir_mode[i]) {
                    case LogStream::DAILY_FILE: {
                        std::time_t t  = static_cast<std::time_t>(p->time.nanoseconds / 1000000000);
                        std::tm     tm = logLocalTime(t);
                        path << std::put_time(&tm, "%Y-%m-%d");
                        break;
                    }
                    case LogStream::SPLIT_BY_SEVERITY: {
                        path << to_string(p->info->severity);
                        break;
                    }
                    case LogStream::SPLIT_BY_CATEGORY: {
                        path << to_category(p->info->category);
                        break;
                    }
                    default: continue;
                }
                last = 1;
            }
            std::cerr << path.str() << std::endl;

            FILE* file = cache.get(path.str());
            fwrite(ss.c_str(), ss.size(), 1, file);
        }
    }

protected:
    zeroerr::String to_string(LogSeverity severity) {
        switch (severity) {
            case INFO_l:  return "INFO";
            case LOG_l:   return "LOG";
            case WARN_l:  return "WARN";
            case ERROR_l: return "ERROR";
            case FATAL_l: return "FATAL";
        }
        return "";
    }

    zeroerr::String to_category(const char* category) {
        zeroerr::String cat = category;
        for (auto& c : cat) {
            if (c == '/') c = split;
        }
        return cat;
    }

    LogStream::DirMode dir_mode[3];
    zeroerr::String        dirpath;
};

class OStreamLogger : public Logger {
public:
    OStreamLogger(std::ostream& os) : os(os) {}

    void flush(DataBlock* msg) override {
        for (auto p = msg->begin(); p < msg->end(); p = moveBytes(p, p->info->size)) {
            os << log_custom_callback(*p, true);
        }
        os.flush();
    }

protected:
    std::ostream& os;
};


LogStream& LogStream::getDefault() {
    static LogStream stream;
    return stream;
}

void LogStream::setFileLogger(zeroerr::String name, DirMode mode1, DirMode mode2, DirMode mode3) {
    if (logger) delete logger;

    if (mode1 == 0 || mode2 == 0 || mode3 == 0)
        logger = new FileLogger(name);
    else {
        LogStream::DirMode dir_mode_group[3] = {mode1, mode2, mode3};

        logger = new DirectoryLogger(name, dir_mode_group);
    }
}

void LogStream::setStdoutLogger() {
    if (logger) delete logger;
    logger = new OStreamLogger(std::cout);
}

void LogStream::setStderrLogger() {
    if (logger) delete logger;
    logger = new OStreamLogger(std::cerr);
}


static LogSeverity LogLevel;

static std::unordered_set<zeroerr::String> LogCategory;
static std::vector<zeroerr::String>        AllLogCategory;


void setLogLevel(LogSeverity level) { LogLevel = level; }

void setLogCategory(const char* categories) {
    LogCategory.clear();
    zeroerr::String str = categories;
    zeroerr::String cat;
    for (auto c : str) {
        if (c == ',') {
            LogCategory.insert(cat);
            cat.clear();
        } else {
            cat.push_back(c);
        }
    }
    if (!cat.empty()) {
        LogCategory.insert(cat);
    }
}

static LogStream::FlushMode saved_flush_mode = LogStream::FlushMode::FLUSH_AT_ONCE;

void suspendLog() {
    saved_flush_mode = LogStream::getDefault().getFlushMode();
    LogStream::getDefault().setFlushMode(LogStream::FLUSH_MANUALLY);
}

void resumeLog() {
    LogStream::getDefault().setFlushMode(saved_flush_mode);
    LogStream::getDefault().flush();
}

#define zeroerr_color(x) (colorful ? x : "")
static zeroerr::String DefaultLogCallback(const LogMessage& msg, bool colorful) {
    std::stringstream ss;
    std::time_t       t  = static_cast<std::time_t>(msg.time.nanoseconds / 1000000000);
    std::tm           tm = logLocalTime(t);

    ss << zeroerr_color(Dim) << '[' << zeroerr_color(Reset);
    switch (msg.info->severity) {
        case INFO_l:  ss << "INFO "; break;
        case LOG_l:   ss << zeroerr_color(FgGreen) << "LOG  " << zeroerr_color(Reset); break;
        case WARN_l:  ss << zeroerr_color(FgYellow) << "WARN " << zeroerr_color(Reset); break;
        case ERROR_l: ss << zeroerr_color(FgRed) << "ERROR" << zeroerr_color(Reset); break;
        case FATAL_l: ss << zeroerr_color(FgMagenta) << "FATAL" << zeroerr_color(Reset); break;
    }
    ss << " " << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");

    zeroerr::String fileName(msg.info->filename);

    auto p = fileName.find_last_of('/');
    if (p != zeroerr::String::npos) fileName = fileName.substr(p + 1);
    auto q = fileName.find_last_of('\\');
    if (q != zeroerr::String::npos) fileName = fileName.substr(q + 1);

    ss << " " << fileName << ":" << msg.info->line;
    ss << zeroerr_color(Dim) << ']' << zeroerr_color(Reset) << "  " << msg.str();
    ss << std::endl;
    return ss.str();
}
#undef zeroerr_color

}  // namespace zeroerr

namespace zeroerr {
LogMessage::LogMessage() {
    time.nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}
}
