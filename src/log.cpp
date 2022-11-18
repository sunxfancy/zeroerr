#include "zeroerr/log.h"

const char* ZEROERR_LOG_CATEGORY = "default";


namespace zeroerr {

thread_local std::vector<IContextScope*> _ZEROERR_G_CONTEXT_SCOPE_VECTOR;


class FileLogger : public Logger {
public:
    FileLogger(std::string name, bool binary = true) {
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
            // TODO: write binary data and string data
            // fwrite(msg, 1, strlen(msg), file);
            fflush(file);
        }
    }
protected:
    FILE* file;
};

class OStreamLogger : public Logger {
public:
    OStreamLogger(std::ostream& os) : os(os) {}

    void flush(DataBlock* msg) override {
        // TODO: write to stream
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
