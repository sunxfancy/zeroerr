快速上手
===============


ZeroErr可以采用两种方式引用，其一是直接下载 `zeroerr.hpp` 文件，该文件打包了所有头文件和实现，您可以把它放到您项目目录中直接引用，并使用 `ZEROERR_IMPLEMENTATION` 宏，在某个引用 `zeroerr.hpp` 的源文件中启用，这样可以把所有实现部分放置到该编译单元中。

```
#define ZEROERR_IMPLEMENTATION
#include "zeroerr.hpp"
```

其二是使用CMake，把本仓库当做一个CMake子项目引用，这样您可以按需引用需要的头文件, 例如，单独引用 `zeroerr/assert.h` 可以引入断言库部分。但您需要在最后链接编译后的库 `libzeroerr.a`


下面我们用一些简单的示例来说明如何使用框架。

## 单元测试

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

- `CHECK_EQ(a, b)` 判断是否相等   `==`
- `CHECK_NE(a, b)` 判断是否不等   `!=`
- `CHECK_LT(a, b)` 判断是否小于   `<`
- `CHECK_GT(a, b)` 判断是否大于   `>`
- `CHECK_LE(a, b)` 判断是否小于等于 `<=`
- `CHECK_GE(a, b)` 判断是否大于等于 `>=`


## log 日志系统

日志系统提供了一组宏：

- `LOG` 表示日常事件，普通记录发生了什么
- `WARN` 表示警告，目前系统中出现了可能需要关注的事件
- `ERROR` 错误，目前系统中出现了非预期的问题
- `FATAL` 致命错误，导致目前系统必须立即停机

这些宏的使用方法较为类似：

```
LOG("message {n1} {n2}", data1, ...)
```

首先是消息的格式化字符串，这个字符串必须是const char*类型的原始字符串，不能是由string转换的，或者自己拼接构造的，因为这个字符串的地址还将被用于索引该LOG发生的位置。

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



`LOG_FIRST` 只记录首次运行到此

```
LOG_FIRST("message {n1} {n2}", data1, ...)
```
