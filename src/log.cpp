#include "zeroerr/log.h"
#include "zeroerr/internal/threadsafe.h"

const char* ZEROERR_LOG_CATEGORY = "default";


namespace zeroerr {

int _ZEROERR_G_VERBOSE = 0;

thread_local std::vector<IContextScope*> _ZEROERR_G_CONTEXT_SCOPE_VECTOR;

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
    // FIXME: this flush did not flush the previous data block
    logger->flush(last);
    last->size = 0;
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
                for (LogMessage* p = (LogMessage*)msg->data; 
                        p < (LogMessage*)&msg->data[msg->size]; p += p->info->size) {
                    auto ss = p->str(false);
                    fwrite(ss.c_str(), ss.size(), 1, file);
                }
            }
            fflush(file);
        }
    }
protected:
    FILE* file;
    bool binary;
};

class OStreamLogger : public Logger {
public:
    OStreamLogger(std::ostream& os) : os(os) {}

    void flush(DataBlock* msg) override {
        for (LogMessage* p = (LogMessage*)msg->data; 
                p < (LogMessage*)&msg->data[msg->size]; p += p->info->size) {
            os << p->str();
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


static LogSeverity                     LogLevel;

static std::unordered_set<std::string> LogCategory;
static std::vector<std::string>        AllLogCategory;


void setLogLevel(LogSeverity level) {
    LogLevel = level;
}

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


}  // namespace zeroerr
