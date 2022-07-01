# ZeroErr

[![Standard](https://img.shields.io/badge/C%2B%2B%2FCUDA-11%2F14%2F17%2F20-blue)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)


Hope you get 0 errors and 0 warnings everyday!

![](./doc/zeroerr.jpg)



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


## Header-only libraries

* dbg
* print (without use extern functions)
* assert
* color (if always enable)
  

