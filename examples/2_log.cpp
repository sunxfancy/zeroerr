#define ZEROERR_IMPLEMENTATION
#include "zeroerr.hpp"

using namespace zeroerr;

TEST_CASE("1. basic log test") {
    LOG("Basic log");
    WARN("Warning log");
    ERR("Error log");
    FATAL("Fatal log");
    
    LOG("log with basic thype {} {} {} {}", 1, true, 1.0, "string");

    std::vector<std::tuple<int, float, std::string>> data = {
        {1, 1.0, "string"}, {2, 2.0, "string"}
    };
    LOG("log with complex type: {data}", data);

    LOG_IF(1==1, "log if condition is true");
    LOG_FIRST(1==1, "log only at the first time condition is true");
    WARN_EVERY_(2, "log every 2 times");
    WARN_IF_EVERY_(2, 1==1, "log if condition is true every 2 times");

    DLOG(WARN_IF, 1==1, "debug log for WARN_IF");
}


struct Expr {
    virtual Expr* Clone() { return new Expr(); };
    virtual ~Expr() {}
};

Expr* parse_the_input(std::string input) {
    return new Expr();
}


Expr* parseExpr(std::string input)
{
    static std::map<std::string, Expr*> cache;
    if (cache.count(input) == 0) {
        Expr* expr = parse_the_input(input);
        cache[input] = expr;
        return expr;
    } else {
        LOG("CacheHit: input = {input}", input);
        return cache[input]->Clone();
    }
}
TEST_CASE("parsing test") {
    zeroerr::suspendLog();
    Expr* e1 = parseExpr("1 + 2");
    // CHECK(LOG_GET(parseExpr, "CacheHit", input, std::string) == std::string{});
    Expr* e2 = parseExpr("1 + 2");
    std::string log = LOG_GET(parseExpr, "CacheHit", input, std::string);
    CHECK(log == "1 + 2"); 
    zeroerr::resumeLog();
}


TEST_CASE("iterate log stream") {
    zeroerr::suspendLog();

    auto& stream = zeroerr::LogStream::getDefault();

    for (auto p = stream.begin("function log {sum}, {i}"); p != stream.end(); ++p) {
        std::cerr << "p.get<int>(\"sum\") = " << p.get<int>("sum") << std::endl;
        std::cerr << "p.get<int>(\"i\") = " << p.get<int>("i") << std::endl;
        CHECK(p.get<int>("sum") == 10);
        CHECK(p.get<int>("i") == 1);
    }

    zeroerr::resumeLog();
}

