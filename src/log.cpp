#include "zeroerr/log.h"
#include "zeroerr/internal/threadsafe.h"

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

static std::string       DefaultLogCallback(const LogMessage& msg, bool colorful);
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
      severity(severity) {
    for (const char* p = message; *p; p++)
        if (*p == '{') {
            const char* q = p + 1;
            while (*q && *q != '}') q++;
            if (*q == '}') {
                std::string N(p + 1, q);
                names[N] = names.size();
                p        = q;
            }
        }
}


constexpr size_t LogStreamMaxSize = 1 * 1024 - 16;

struct DataBlock {
    ZEROERR_ATOMIC(size_t) size;
    DataBlock* next = nullptr;
    char       data[LogStreamMaxSize];

    DataBlock() : size(0) {}

    LogMessage* begin() { return (LogMessage*)data; }
    LogMessage* end() { return (LogMessage*)&data[size]; }
};

LogStream::LogStream() {
    first = m_last = new DataBlock();
    prepare        = new DataBlock();
#ifndef ZEROERR_NO_THREAD_SAFE
    mutex = new std::mutex();
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
    ZEROERR_LOCK(*mutex);
    auto* last = m_last.load();
    if (last->size + size > LogStreamMaxSize) {
        if (flush_mode == FLUSH_WHEN_FULL) {
            logger->flush(last);
            last->size = 0;
        } else {
            last->next = new DataBlock();
            last       = last->next;
        }
    }
    void* p = last->data + last->size;
    last->size += size;
    return p;
}

void* LogStream::alloc_block_lockfree(unsigned size) {
    if (size > LogStreamMaxSize) {
        throw std::runtime_error("LogStream::push: size > LogStreamMaxSize");
    }
    DataBlock* last = m_last.load();
    while (true) {
        size_t p = last->size.load();
        if (p <= (LogStreamMaxSize - size)) {
            if (last->size.compare_exchange_strong(p, p + size)) return last->data + p;
        } else {
            if (m_last.compare_exchange_strong(last, prepare)) {
                if (flush_mode == FLUSH_WHEN_FULL) {
                    logger->flush(last);
                    last->size = 0;
                    prepare    = last;
                } else {
                    prepare->next = last;
                    prepare       = new DataBlock();
                }
            }
        }
    }
}

void LogStream::flush() {
    ZEROERR_LOCK(*mutex);
    DataBlock* last = m_last.load();
    for (DataBlock* p = first; p != last; p = p->next) {
        logger->flush(p);
        delete p;
    }
    logger->flush(last);
    last->size = 0;
    first      = last;
}

static LogMessage* moveBytes(LogMessage* p, unsigned size) {
    char* src = (char*)p;
    char* dst = src + size;
    return (LogMessage*)dst;
}

void* LogStream::getRawLog(std::string func, unsigned line, std::string name) {
    for (DataBlock* p = first; p; p = p->next)
        for (auto q = p->begin(); q < p->end(); q = moveBytes(q, q->info->size))
            if (line == q->info->line && func == q->info->function) return q->getRawLog(name);
    return nullptr;
}

void* LogStream::getRawLog(std::string func, std::string msg, std::string name) {
    for (DataBlock* p = first; p; p = p->next)
        for (auto q = p->begin(); q < p->end(); q = moveBytes(q, q->info->size))
            if (msg == q->info->message && func == q->info->function) return q->getRawLog(name);
    return nullptr;
}

LogIterator::LogIterator(LogStream& stream) : p(stream.first), q(stream.first->begin()) {}

LogIterator& LogIterator::operator++() {
    if (q < p->end()) {
        q = moveBytes(q, q->info->size);
        return *this;
    } else {
        p = p->next;
        if (p) {
            q = p->begin();
            return *this;
        } else {
            q = nullptr;
            return *this;
        }
    }
}

class FileLogger : public Logger {
public:
    FileLogger(std::string name) { file = fopen(name.c_str(), "w"); }
    ~FileLogger() {
        if (file) fclose(file);
    }
    void flush(DataBlock* msg) override {
        if (file) {
            for (auto p = msg->begin(); p < msg->end(); p = moveBytes(p, p->info->size)) {
                auto ss = log_custom_callback(*p, false);
                fwrite(ss.c_str(), ss.size(), 1, file);
            }
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

static void make_dir(std::string path) {
    size_t pos = 0;
    do {
        pos             = path.find_first_of(split, pos + 1);
        std::string sub = path.substr(0, pos);
#ifdef _WIN32
        CreateDirectory(sub.c_str(), NULL);
#else
        struct stat st;
        if (stat(sub.c_str(), &st) == -1) {
            mkdir(sub.c_str(), 0755);
        }
#endif
    } while (pos != std::string::npos);
}

struct FileCache {
    std::map<std::string, FILE*> files;
    ~FileCache() {
        for (auto& p : files) {
            fclose(p.second);
        }
    }

    FILE* get(const std::string& name) {
        auto it = files.find(name);
        if (it != files.end()) return it->second;
        auto        p    = name.find_last_of(split);
        std::string path = name.substr(0, p);
        make_dir(path.c_str());

        FILE* file = fopen(name.c_str(), "w");
        if (file) {
            files[name] = file;
            return file;
        }
        return nullptr;
    }
};


class DirectoryLogger : public Logger {
public:
    DirectoryLogger(std::string path, LogStream::DirMode dir_mode[3]) : dirpath(path) {
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
                        std::time_t t  = std::chrono::system_clock::to_time_t(p->time);
                        std::tm     tm = *std::localtime(&t);
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
    std::string to_string(LogSeverity severity) {
        switch (severity) {
            case INFO_l:  return "INFO";
            case LOG_l:   return "LOG";
            case WARN_l:  return "WARN";
            case ERROR_l: return "ERROR";
            case FATAL_l: return "FATAL";
        }
        return "";
    }

    std::string to_category(const char* category) {
        std::string cat = category;
        for (auto& c : cat) {
            if (c == '/') c = split;
        }
        return cat;
    }

    LogStream::DirMode dir_mode[3];
    std::string        dirpath;
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

void LogStream::setFileLogger(std::string name, DirMode mode1, DirMode mode2, DirMode mode3) {
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

static std::unordered_set<std::string> LogCategory;
static std::vector<std::string>        AllLogCategory;


void setLogLevel(LogSeverity level) { LogLevel = level; }

void setLogCategory(const char* categories) {
    LogCategory.clear();
    std::string str = categories;
    std::string cat;
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

static LogStream::FlushMode saved_flush_mode;

void suspendLog() {
    saved_flush_mode = LogStream::getDefault().getFlushMode();
    LogStream::getDefault().setFlushMode(LogStream::FLUSH_MANUALLY);
}

void resumeLog() {
    LogStream::getDefault().setFlushMode(saved_flush_mode);
    LogStream::getDefault().flush();
}

#define zeroerr_color(x) (colorful ? x : "")
static std::string DefaultLogCallback(const LogMessage& msg, bool colorful) {
    std::stringstream ss;
    std::time_t       t  = std::chrono::system_clock::to_time_t(msg.time);
    std::tm           tm = *std::localtime(&t);

    ss << zeroerr_color(Dim) << '[' << zeroerr_color(Reset);
    switch (msg.info->severity) {
        case INFO_l:  ss << "INFO "; break;
        case LOG_l:   ss << zeroerr_color(FgGreen) << "LOG  " << zeroerr_color(Reset); break;
        case WARN_l:  ss << zeroerr_color(FgYellow) << "WARN " << zeroerr_color(Reset); break;
        case ERROR_l: ss << zeroerr_color(FgRed) << "ERROR" << zeroerr_color(Reset); break;
        case FATAL_l: ss << zeroerr_color(FgMagenta) << "FATAL" << zeroerr_color(Reset); break;
    }
    ss << " " << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");

    std::string fileName(msg.info->filename);

    auto p = fileName.find_last_of('/');
    if (p != std::string::npos) fileName = fileName.substr(p + 1);
    auto q = fileName.find_last_of('\\');
    if (q != std::string::npos) fileName = fileName.substr(q + 1);

    ss << " " << fileName << ":" << msg.info->line;
    ss << zeroerr_color(Dim) << ']' << zeroerr_color(Reset) << "  " << msg.str();
    ss << std::endl;
    return ss.str();
}
#undef zeroerr_color

}  // namespace zeroerr
