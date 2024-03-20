# ZeroErr

[![Standard](https://img.shields.io/badge/C%2B%2B%2FCUDA-11%2F14%2F17%2F20-blue)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization) [![download](https://img.shields.io/badge/-Download-brightgreen)](https://raw.githubusercontent.com/sunxfancy/zeroerr/master/zeroerr.hpp) [![Chinese-Readme](https://img.shields.io/badge/%E4%B8%AD%E6%96%87-Readme-blue)](./Readme.md)


Hope you get 0 errors and 0 warnings everyday!

![](./docs/fig/zeroerr.jpg)


ZeroErr is a smart assertion library, a lightweight unit testing framework and a structure logging framework. It integrates those features and provided an unite and clear interface for seperate using or joint using. 

[English Documentation](https://sunxfancy.github.io/zeroerr/en/) | [项目文档](https://sunxfancy.github.io/zeroerr/zh/)

Note: The project is currently in the experimental stage, and the API may change significantly. It is not recommended to use it in a production environment.


## Why we need another unit testing framework

The current popular unit testing frameworks, e.g. Catch2, doctest, Boost.Test and cpputest are mature and well-established which covers common cases during development. The logger libraries like glog and spdlog are also easy to use. However, there are still some issues:

### 1. Generic Printing

Most unit testing frameworks and logger libraries can not provide a generic printing for user customized type. Especially, when using containers, struct and pointers (including smart pointers), user have to manualy write code to generate the log message or print those information during unit testing failed cases. 

This library `zeroerr` gives you an ability to print generically for all types:

```c++
TEST_CASE("Try logging") {
    std::vector<int> data = {1, 2, 3};
    LOG_IF(1 == 1, "data = {data}", data);
}
```

Similar to other C++ unit testing frameworks, `zeroerr` will convert this piece of code into a function and register it to automatically run once you link the main function and the library. Here, we can log the data in `vector` template directly without writing any code. 

![case1](docs/fig/case1.png)

For the custom struct type with override `std::ostream& operator<<(std::ostream&, Type)` stream output, you can use it not only for this type but also all contains using this type, including multiple recurisve contains:

```c++
struct Node {
    std::string name;
    int id;
};

std::ostream& operator<<(std::ostream& out, Node n) {
    out << n.id << '.' << n.name;
    return out;
}

TEST_CASE("Try logging with custom type") {
    std::map<std::string, std::vector<Node>> data = {
        {"user1", {{"a",1}, {"b",2}}}, {"user2", {{"c",3}, {"d",4}}}
    };
    LOG("data = {data}", data);
}
```

![case2](docs/fig/case2.png)

Of cause, in many cases, some third-party libraries may not use `<<` operators. For those cases, we can write own rules to create a generic way for printing. For example, LLVM `llvm::Function*` type can not be streamed into std::ostream, we can write code to handle it. However, it will be more simple if we can write a rule for all the sub-classes of `llvm::Value` and `llvm::Type` since we can call the `print` method to print the output. Here we use a `dbg` marco defined in `zeroerr` to quickly print any type. This is very similar to the `dbg` marco in rust.


```c++
namespace zeroerr { // must defined in namespace zeroerr

template <typename T>
typename std::enable_if<
    std::is_base_of<llvm::Value, T>::value || std::is_base_of<llvm::Type, T>::value, void>::type
PrinterExt(Printer& P, T* s, unsigned level, const char* lb, rank<2>) {
    if (s == nullptr) {
        P.os << P.tab(level) << "nullptr" << lb;
    } else {
        llvm::raw_os_ostream os(P.os);
        s->print(os);
    }
}
}

TEST_CASE("customize printing of LLVM pointers") {
    llvm::LLVMContext        context;
    std::vector<llvm::Type*> args   = {llvm::Type::getInt32Ty(context)};
    llvm::Module*            module = new llvm::Module("test_module", context);

    auto* f =
        llvm::Function::Create(llvm::FunctionType::get(llvm::Type::getVoidTy(context), args, false),
                               llvm::GlobalValue::ExternalLinkage, "test", module);
    dbg(dbg(f)->getType());
}
```

This functin `PrintExt` will match all the class who's base class is `Value` and `Type`. Then, it will create a stream ``llvm::raw_os_ostream` for output.

![case3-llvm](./docs/fig/case3.png)

### 2. Joint using of assert, log and unit testing

If you use one logging framework, an unit testing framework and a smart assertion libary, you can not easily combine them together. In `zeroerr`, if an assertion is failed, the logger will recevie an event and stored the event in your log file. If you are using an assertion in unit testing, the assertion failure, logged fatal events can be recorded and reported.

```c++
int fib(int n) {
    REQUIRE(n >= 0, "n must be non-negative");
    REQUIRE(n < 20, "n must be less than 20");
    if (n <= 2) {
        return 1;
    }
    return fib(n - 1) + fib(n - 2);
}

TEST_CASE("fib function test") {
    CHECK(fib(0) == 0);
    CHECK(fib(1) == 1);
    CHECK(fib(2) == 1);
    CHECK(fib(3) == 2);
    CHECK(fib(4) == 3);
    CHECK(fib(5) == 5);
    CHECK(fib(20) == 6765);
}
```

![joint1](docs/fig/joint1.png)

Further more, the unit testing can check the logged result if it matches the previous running result (a golden file) to avoid writing any code in the test case.

```c++
TEST_CASE("match ostream") {
    // match output can be done in the following workflow
    // 1. user mark the test case which are comparing output use 'ZEROERR_HAVE_SAME_OUTPUT'
    // 2. If the output is not exist, the result has been used as a correct verifier.
    // 3. If the output is exist, compare with it and report error if output is not match.
    std::cerr << "a = 100" << std::endl;

    ZEROERR_HAVE_SAME_OUTPUT;
}
```

Once you set `ZEROERR_HAVE_SAME_OUTPUT` marco, the system will check the output stream and save the first run result into a file. Then, the next run will compare the result to see if it the same. (Currently experimental)

Finally, for the log system, the unit testing can access the log data to ensure that the function has executed the expected logic and results.

```c++
118 static void function() {
119    LOG("function log {i}", 1);  
120    LOG("function log {sum}, {i}", 10, 1);
121 }
...

TEST_CASE("access log in Test case") {
    zeroerr::suspendLog();
    function();
    CHECK(LOG_GET(function, 119, i, int) == 1);
    CHECK(LOG_GET(function, 120, sum, int) == 10);
    CHECK(LOG_GET(function, 120, i, int) == 1);
    zeroerr::resumeLog();
}
```

In order to access the log, we need to pause the log system first, to avoid the data being output to the file, then call the function, access the data in the log through the `LOG_GET` macro, and finally resume the log system. (Currently experimental, only the first call of each log point can be accessed)

## 3. Fuzzing Support

Most Unit Testing frameworks do not support fuzzing. However, it's a powerful feature to automatically detect faults in the software and can greatly reduce the work to write test cases.

Different than other fuzzing framework, `zeroerr` can also support logging and assertion in the code, so the fuzzing result not only contains corpus but also with the logging and assertion information.

Here is an example of using `zeroerr` to do structured fuzzing:

```c++
FUZZ_TEST_CASE("fuzz_test") {
    LOG("Run fuzz_test");
    FUZZ_FUNC([=](int k, std::string num) {
        int t = atoi(num.c_str());
        LOG("k: {k}, num:{num}, t: {t}", k, num, t);
        REQUIRE(k == t);
    })
        .WithDomains(InRange<int>(0, 10), Arbitrary<std::string>())
        .WithSeeds({{5, "Foo"}, {10, "Bar"}})
        .Run(10);
}
```

Inspired by [fuzztest](https://github.com/google/fuzztest), Domain is a concept to specify the input data range (or patterns) for the target function. Here, we use `InRange` to specify the range of `k` is 0 to 10, and `Arbitrary` to specify the data of `num` can be any random string. Then, we use `WithSeeds` to specify the initial seeds for the fuzzing. 

The macro `FUZZ_TEST_CASE` will generate a test case which can connect with `libFuzzer` to run the fuzzing. Finally, we use `Run(10)` to call `libFuzzer` to run the target for 10 times.

To build the test case with fuzzing, you need to use `clang++` to compile the code and with `-fsanitize=fuzzer-no-link` and link the `-lclang_rt.fuzzer_no_main-x86_64` which is a version of libFuzzer without main function. You can find this runtime library by calling `clang++ -print-runtime-dir`. Here is the complete command to build the test case with fuzzing support:

```bash
clang++ -std=c++11 -fsanitize=fuzzer-no-link -L=`clang++ -print-runtime-dir` -lclang_rt.fuzzer_no_main-x86_64  -o test_fuzz test_fuzz.cpp 
```


## Other Good Features


Here are a list of features we provided:

1. Partially include
You can only include what you need. If you need only assertion but no unit testing, no problem.

2. Optional thread safety 
You can choose to build with/without thread safety. For some simple single thread program, log is no need to be multithread safed.

3. Fastest log
Using a lock-free queue for logging and multiple level of log writing policies. You can choose to only write to disk with the most important events.

4. Customized print / log / assert printing format
You can customize your printing format for everything. There is a templated callback function for the printing.

5. Quickly debug something
You can use dbg macro to quickly see the output, it can be applied to any expression.

6. Colorful output
You can have default colorful output to terminal and no color for file output

7. Print struct/stl/pointers without any extra code

8. Doctest like assertion and unit test feature
You can use your unit test as a documentation of function behavior. The output of unittest can be a documented report.

9. Lazy logging for assertion
After assertion failed, the logging result will print automatically even if you didn't redirect to your error stream

10. Logging Category 
Logging information can have customized category and only display one categroy based on your assertion or configuration

11. Logging for Unit Testing
You can use a correct logging result as your unit testing golden file. So you just need to manually verify your log once and save it. The unit testing framework will use the golden file to verify your unit testing result.

12. Structured Logging
We can support output structured information directly into plain text or lisp format (json, logfmt, or other custom format should be the next step to support)

13. Automatic Tracing with logging
While logging at the end, we can record the time consuming for this function.

## Header-only libraries

* dbg
* print (without use extern functions)
* assert
* color (if always enable)


## The logo generation

Thanks to the `tiv` tool:
https://github.com/stefanhaustein/TerminalImageViewer