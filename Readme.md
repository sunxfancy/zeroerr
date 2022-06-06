# ZeroErr

Hope you get 0 errors and 0 warnings everyday!

![](./zeroerr.jpg)



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

8. Doctest
You can use your unit test as a documentation of function behavior. The output of unittest can be a documented report.
