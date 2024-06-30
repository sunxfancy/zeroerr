快速上手
===============
[TOC]

ZeroErr可以采用两种方式引用，其一是直接下载 `zeroerr.hpp` 文件，该文件打包了所有头文件和实现，您可以把它放到您项目目录中直接引用，并使用 `ZEROERR_IMPLEMENTATION` 宏，在某个引用 `zeroerr.hpp` 的源文件中启用，这样可以把所有实现部分放置到该编译单元中。

```
#define ZEROERR_IMPLEMENTATION
#include "zeroerr.hpp"
```

其二是使用CMake，把本仓库当做一个CMake子项目引用，这样您可以按需引用需要的头文件, 例如，单独引用 zeroerr/assert.h 可以引入断言库部分。但您需要在最后链接编译后的库 `libzeroerr.a`


下面我们用一些简单的示例来说明如何使用框架。

### 单元测试和断言

`TEST_CASE` 宏是最基础的定义单元测试的宏，如果您熟悉 catch2 或 doctest, 您应该对这种写法非常熟悉。我们给该测试一个名字，然后在接下来的函数体中编写测试代码。

```
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
}
```

`CHECK` 宏提供了检测一个表达式的功能，由于我们这里 `fib(0)` 没有计算正确，于是我们会获得如下输出：

```
[WARN  2023-05-10 16:15:44 1.basic.cpp:44]   Assertion Failed:
        fib(0) == 0  expands to  1 == 0
    (/mnt/SSD/Workspace/zeroerr/examples/1.basic.cpp:44)
```

这里在出错后，会自动打印出，测试时间、出错的文件，行号，断言表达式，以及展开后是什么样子。这些信息能够直观的让人明白哪里出错了，并且可以找到出错点进行排查。

类似的宏还有 `REQUIRE` 和 `ASSERT`，但这三者异常等级不同，`CHECK`表示检测，如果出错后会继续执行后面代码。`REQUIRE` 表示较为严重的错误，出错后没有必要继续执行，会抛出一个异常，并在log中显示Error等级的信息。`ASSERT`表示致命的错误，如果发生，则代码无法向下执行，会导致程序崩溃的异常。

以上三个宏可以直接检测简单的表达式，但其包含一定的开销，我们还提供更轻量的一组宏来实现同样的功能（`REQUIRE` 和 `ASSERT` 同理）：

- CHECK_EQ(a, b) 判断是否相等   `==`
- CHECK_NE(a, b) 判断是否不等   `!=`
- CHECK_LT(a, b) 判断是否小于   `<`
- CHECK_GT(a, b) 判断是否大于   `>`
- CHECK_LE(a, b) 判断是否小于等于 `<=`
- CHECK_GE(a, b) 判断是否大于等于 `>=`


所有的断言宏，不但可以在单元测试中使用，也可以在任意函数中使用。


### log 日志系统

日志系统提供了一组宏：

- `LOG` 表示日常事件，普通记录发生了什么
- `WARN` 表示警告，目前系统中出现了可能需要关注的事件
- `ERROR` 错误，目前系统中出现了非预期的问题
- `FATAL` 致命错误，导致目前系统必须立即停机

这些宏的使用方法较为类似：

```
LOG("message {n1} {n2}", data1, ...)
```

本系统提供的是一种结构化的日志记录方式，使用消息 + 参数的方式来表示一个事件。我们并不会立刻将日志格式化成字符串，而是将参数记录下来，等到需要输出时，再进行格式化，或者直接将二进制的参数保存下来。这样可以提高日志的效率，并且给我们在记录日志后，用编程的方式访问日志数据的可能性。

首先是消息的格式化字符串，这个字符串必须是const char*类型的原始字符串，不能是由string转换的，或者自己拼接构造的，因为这个字符串的地址还将被用于索引该LOG发生的位置。同时，要注意，所有的参数必须是可以拷贝的，这样才能保证在日志输出时不出现问题。


`{n1}` 表示一个任意类型的参数，名称是`n1`。这种设计的原因是，log一般用来记录一些事件的发生，如果我们立即格式化字符串将log写入文件，往往会消耗一定时间，然而，如果我们只是将参数记录下来，那么效率就能大大提升，设置一个名称，也可以方便对log的数据进行检索，从而在系统中快速找出需要的log条目。


`LOG_IF` 条件log，使用方法

```
LOG_IF(condition, "message {n1} {n2}", data1, ...)
```

`LOG_EVERY_` 每N次执行到此，log一下，会在循环 1, 1+N, 1+2N 进行log

```
LOG_EVERY_(N, "message {n1} {n2}", data1, ...)
```



`LOG_IF_EVERY_` 类似于上两条结合在一起

```
LOG_IF_EVERY_(N, condition, "message {n1} {n2}", data1, ...)
```



`LOG_FIRST` 只记录首次运行到此的事件信息

```
LOG_FIRST("message {n1} {n2}", data1, ...)
```


`LOG` 系列的宏会自动将日志写到标准错误流（stderr）中，如果您希望其输出到文件，可以使用 `setFileLogger` 函数进行设置：

```
zeroerr::LogStream::getDefault().setFileLogger("log.txt");
```

您也可以在任意时刻将日志再重新定向到标准错误流：

```
zeroerr::LogStream::getDefault().setStderrLogger();
```


您也可以创建多个`LogStream`对象，在LOG时指定输出流，每个对象可以有自己的日志文件，这样可以实现多个日志文件的输出。

```
zeroerr::LogStream log1;
log1.setFileLogger("log1.txt");

LOG("message {n1} {n2}", log1, data1, data2);
```

### 开启Fuzzing支持

Fuzzing是一种常用的自动化测试手段，通过不断随机生成测试数据，来测试程序的稳定性。ZeroErr提供了一种简单的libfuzzer集成方式，只需要使用 `clang++` 编译代码，并使用 `-fsanitize=fuzzer-no-link` 并链接 `-lclang_rt.fuzzer_no_main-x86_64`，这是一个没有main函数的libFuzzer版本。您可以通过调用 `clang++ -print-runtime-dir` 来找到这个运行时库。

```
> clang++ -print-runtime-dir                                                                                                            /mnt/h/Workspace/zeroerr(dev✗)@xiaofan-pc
/usr/lib/llvm-14/lib/clang/14.0.0/lib/linux
```

类似于 `TEST_CASE` 宏，我们提供了 `FUZZ_TEST_CASE` 宏，用于定义一个fuzzing测试用例，您可以在其中使用 `FUZZ_FUNC` 宏，来定义一个fuzzing测试的函数，该函数接受一个或多个参数，这些参数可以通过 `WithDomains` 来指定，`WithSeeds` 来指定初始种子，`Run` 来指定运行次数。


```
// test_fuzz.cpp 

#define ZEROERR_IMPLEMENTATION
#include "zeroerr.hpp"


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


以下是带有fuzzing支持的测试用例的完整构建命令：

```
clang++ -std=c++11 -I<path of zeroerr.hpp> -fsanitize=fuzzer-no-link -L=`clang++ -print-runtime-dir` -lclang_rt.fuzzer_no_main-x86_64  -o test_fuzz test_fuzz.cpp 
```



