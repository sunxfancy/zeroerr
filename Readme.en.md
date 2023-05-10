# ZeroErr

[![Standard](https://img.shields.io/badge/C%2B%2B%2FCUDA-11%2F14%2F17%2F20-blue)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization) [![download](https://img.shields.io/badge/-Download-brightgreen)]() [![Chinese-Readme](https://img.shields.io/badge/%E4%B8%AD%E6%96%87-Readme-blue)](./Readme.md)


Hope you get 0 errors and 0 warnings everyday!

![](./doc/fig/zeroerr.jpg)



ZeroErr is a smart assert library, a lightweight unit test framework and a quick logging framework. 

Using ZeroErr, you can catch your assert error, log fatal event in the unit testing.
The fatal condition will be recorded and printed.


1. Minimal Requirement
You can only include what you need. If you need assert but no unit testing, no problem.

2. Optional thread safety 
You can choose to build with/without thread safety. For some simple single thread program, log is no need to be multithread safed.

3. Fastest log
Using a lock-free queue for logging and multiple level of log writing policies. You can choose to only write to disk with the most important events.

4. Customized print / log / assert printing format
You can customize your printing format for everything. There is a callback function for the printing.

5. Quickly debug something
You can use dbg macro to quickly see the output, it can print the expression also.

6. Colorful output
You can have default colorful output to terminal and no color for file 

7. Print struct/stl/special library data structure

8. Doctest like assertion and unit test feature
You can use your unit test as a documentation of function behavior. The output of unittest can be a documented report.

9. Lazy logging for assertion
After assertion failed, the logging result will print automatically even if you didn't redirect to your error stream

10. Logging Category 
Logging information can have customized category and only display one categroy based on your assertion or configuration

11. Logging for Unit Testing
You can use a correct logging result as your unit testing comparsion. So you just need to manually verify your log once and setup it as baseline comparsion. The unit testing framework will use that as the result to verify unit testing

12. Structured Logging
We can support output structured information directly into plain text, json, logfmt, or other custom format

13. Automatic Tracing with logging
While logging at the end, we can record the time consuming for this function.

## Header-only libraries

* dbg
* print (without use extern functions)
* assert
* color (if always enable)


## How to use 




## Documentations







## The logo generation

Thanks to the `tiv` tool:
https://github.com/stefanhaustein/TerminalImageViewer