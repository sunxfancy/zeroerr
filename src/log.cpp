#include "zeroerr/log.h"
#include "zeroerr/internal/threadsafe.h"

const char* ZEROERR_LOG_CATEGORY = "default";


namespace zeroerr {

int _ZEROERR_G_VERBOSE = 0;

thread_local std::vector<IContextScope*> _ZEROERR_G_CONTEXT_SCOPE_VECTOR;

static std::string       DefaultLogCallback(const LogMessage& msg, bool colorful);
static LogCustomCallback log_custom_callback = DefaultLogCallback;
void setLogCustomCallback(LogCustomCallback callback) { log_custom_callback = callback; }


LogStream::LogStream() {
    first = last = new DataBlock();
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

void LogStream::flush() {
    ZEROERR_LOCK(*mutex);
    for (DataBlock* p = first; p != last; p = p->next) {
        logger->flush(p);
        delete p;
    }
    logger->flush(last);
    last->size = 0;
    first = last;
}

static LogMessage* moveBytes(LogMessage* p, unsigned size) {
    char* src = (char*)p;
    char* dst = src + size;
    return (LogMessage*)dst;
}

void* LogStream::getRawLog(std::string func, unsigned line, std::string name) {
    for (DataBlock* p = first; p; p = p->next)
        for (LogMessage* q = (LogMessage*)p->data; q < (LogMessage*)&p->data[p->size];
             q = moveBytes(q, q->info->size))
            if (line == q->info->line && func == q->info->function)
                return q->getRawLog(name);
    return nullptr;
}

class FileLogger : public Logger {
public:
    FileLogger(std::string name, bool binary = true) : binary(binary) {
        if (binary) {
            file = fopen(name.c_str(), "wb");
        } else {
            file = fopen(name.c_str(), "w");
        }
    }
    ~FileLogger() {
        if (file) fclose(file);
    }
    void flush(DataBlock* msg) override {
        if (file) {
            if (binary) {
                // TODO: Design a binary format, currently, it can not work
                fwrite(msg->data, msg->size, 1, file);
            } else {
                for (LogMessage* p = (LogMessage*)msg->data; p < (LogMessage*)&msg->data[msg->size];
                     p = moveBytes(p, p->info->size)) {
                    auto ss = log_custom_callback(*p, false);
                    fwrite(ss.c_str(), ss.size(), 1, file);
                }
            }
            fflush(file);
        }
    }

protected:
    FILE* file;
    bool  binary;
};

class OStreamLogger : public Logger {
public:
    OStreamLogger(std::ostream& os) : os(os) {}

    void flush(DataBlock* msg) override {
        for (LogMessage* p = (LogMessage*)msg->data; p < (LogMessage*)&msg->data[msg->size];
             p = moveBytes(p, p->info->size)) {
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

void LogStream::setBinFileLogger(std::string name) {
    if (logger) delete logger;
    logger = new FileLogger(name);
}

void LogStream::setFileLogger(std::string name) {
    if (logger) delete logger;
    logger = new FileLogger(name, false);
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
void                        suspendLog() {
    saved_flush_mode                   = LogStream::getDefault().flush_mode;
    LogStream::getDefault().flush_mode = LogStream::FLUSH_MANUALLY;
}

void resumeLog() {
    LogStream::getDefault().flush_mode = saved_flush_mode;
    LogStream::getDefault().flush();
}

#define zeroerr_color(x) (colorful ? x : "")
static std::string DefaultLogCallback(const LogMessage& msg, bool colorful) {
    std::stringstream ss;
    std::time_t       t  = std::chrono::system_clock::to_time_t(msg.time);
    std::tm           tm = *std::localtime(&t);

    ss << zeroerr_color(Dim) << '[' << zeroerr_color(Reset);
    switch (msg.info->severity) {
        case INFO_l: ss << "INFO "; break;
        case LOG_l: ss << zeroerr_color(FgGreen) << "LOG  " << zeroerr_color(Reset); break;
        case WARN_l: ss << zeroerr_color(FgYellow) << "WARN " << zeroerr_color(Reset); break;
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


}  // namespace zeroerr
