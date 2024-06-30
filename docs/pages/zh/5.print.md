Printer MultiFunctor
====================
[TOC]

`Printer` 是一个用来打印任意类型数据的模板类。本身是一个有状态的MultiFunctor, 可以打印到`std::ostream` 或者 `std::stringstream`。

### 构造函数

您可以选择打印到内置的 `stringstream` 或者您传入的一个 `std::ostream`。

```c++
Printer();  // print to std::stringstream
Printer(std::ostream& os); // print to 'os'
```
如果想快速获得一个能打印到标准输出流的对象，可以调用如下方法：

```c++
extern Printer& getStdoutPrinter();
extern Printer& getStderrPrinter();
```

### 打印任意类型

仿函数 `print(...)` 可以传入任意个任意不同类型的参数，他们将被依次打印。


Example:
```c++
int main() {
    auto print = zeroerr::getStderrPrinter();
    int  a = 1;
    int& b = a;
    int* c = &a;
    print(a, b, c);
    return 0;
}
```

Output:
```
1 1 <int* at 0x7ffebc813ed4>
```

同时支持 STL container:

```c++
int main() {
    auto print = zeroerr::getStderrPrinter();
    std::vector<int> vec = {1, 5, 2};
    auto get_student = [](int id) {
        if (id == 0) return std::make_tuple(3.8, 'A', "Lisa Simpson");
        if (id == 1) return std::make_tuple(2.9, 'C', "Milhouse Van Houten");
        if (id == 2) return std::make_tuple(1.7, 'D', "Ralph Wiggum");
        throw std::invalid_argument("id");
    };
    print(a, b, c);
    print({get_student(0), get_student(1), get_student(2)});
    return 0;
}
```
Output:
```
[1, 5, 2] 
{
  (3.8, A, Lisa Simpson)
  (2.9, C, Milhouse Van Houten)
  (1.7, D, Ralph Wiggum)
} 
```

也可以方便的打印智能指针:

```c++
std::shared_ptr<int> p(new int(42));
print(p);

std::unique_ptr<int> up(new int(42));
print(std::move(up));
```

Output:
```
<std::shared_ptr<int> at 0x555e1f936140> 
<std::unique_ptr<int, std::default_delete<int> > at 0x555e1f931100> 
```


### 不同类型的优先级规则

该函数在打印不同类型元素时，是根据不同的类型，选取不同的优先级规则，例如，一个 string 类型，是container类型，也是Object类型，但因为std::string 有专属的规则在level4， 所以会调用专属规则进行打印。

| level | types                                                                             |
| ----- | --------------------------------------------------------------------------------- |
| 5     | User Customized Rules                                                             |
| 4     | std::string,  std::complex,  std::map                                             |
| 3     | std::share_ptr, std::unique_ptr, std::weak_ptr, std::tuple, Array                 |
| 2     | container(who has begin, end methods),   streamable types(with << operator), bool |
| 1     | char, wchar, POD type(need PFR library)                                           |
| 0     | Object, C pointer, integer, float                                                 |

## 配置参数

这个仿函数可以被配置多种不同的运行模式:
1. 紧凑模式 - 所有元素不换行，单行打印，如 `{"a" : 1, "b" : 2}`
   ```c++
   print.isCompact = true;
   ```
2. 彩色输出模式 
   ```c++
   print.isColorful = true;
   ```
3. 引用模式，如果启用时字符串会加引号, `"a"`
   ```c++
   print.isQuoted = true;
   ```
4. 缩进 - 默认是 `2`.
   ```c++
   print.indent = 4
   ```
5. 换行符，windows下您可以调整 - 默认是 `\n`.
   ```c++
   print.line_break = "\r\n"; // for windows
   ```

### 在流输出中打印


```c++
Printer print(); // print to stringstream
std::cerr << print("hello world") << std::endl;
```

### 自定义规则

打印规则是可以自定义的，并且会在最高优先级上。创建一个 `PrinterExt` function templates：

```c++
template <typename T> typename std::enable_if< <condition>, void>::type
zeroerr::PrinterExt(Printer& P, <type of Input>, unsigned level, const char* lb, rank<N>);
```

如下是一个LLVM中指针元素的打印规则，支持 `llvm::Value`, `llvm::Type` 以及当前要打印的具体类型 `llvm::Function`:

```c++
namespace zeroerr{
// This will create a special rule for printing pointers from LLVM value
template <typename T>
typename std::enable_if<
    std::is_base_of<llvm::Value, T>::value || std::is_base_of<llvm::Type, T>::value, void>::type
PrinterExt(Printer& P, T* s, unsigned level, const char* lb, rank<1>) {
    if (s == nullptr) {
        P.os << P.tab(level) << "nullptr" << lb;
    } else {
        llvm::raw_os_ostream os(P.os);
        s->print(os);
    }
}


// Even though llvm::Function is a subclass of `llvm::Value`, but this rule is in rank 2. 
// So it will cover the basic rule. All the pointer base of `llvm::Function` will call 
// this function to print.
template <typename T>
typename std::enable_if<std::is_base_of<llvm::Function, T>::value, void>::type PrinterExt(
    Printer& P, T* s, unsigned level, const char* lb, rank<2>) {
    if (s == nullptr) {
        P.os << P.tab(level) << "nullptr" << lb;
    } else {
        llvm::raw_os_ostream os(P.os);
        os << P.tab(level) << "Function " << FgCyan << *s << Reset;
    }
}
} // namespace zeroerr
```