Mix Assertion, Logging, Unit Testing and Fuzzing 
=================================================

## Issues in Unit Tesing

Almost every project needs unit testing. This is most important way to build a strong software but it sometimes may be hard to check boundary cases. Let's consider an example:

```c++

bool target(int x, int y) 
{
    if (x < 0) {
        LOG(ERROR) << "The x can not smaller than 0";
        return false;
    }
    if (x+y < 0) {
        LOG(ERROR) << "x+y = " << x+y << " , which can not smaller than 0";
        return false;
    }
}
```





It may also conflict with assertion in the function.

```c++

void target(int x)
{
    assert(x != 0);
}

TEST_CASE("A unit test case")
{
    SUB_CASE("case1") {
        target(0);
    }

    SUB_CASE("case2") {
        target(1);
    }
}
```

All assertions follow a simple rule - once failed, exit. This will block the following test cases execution. This tell us that assertion should be awared by the unit testing, or at least, it should throw an exception instead exit the program.






## Design a Smart Assertion Library

Have you even though the assertion macro is not power enough? For example, when you write an assertion, you may want to have some message to make it clear. 

```c++
void compute(int x)
{
    assert(x != -1  && "x can not be -1 since ...");
    assert(x > 0 && "x must larger than 0 since ...");
}
```

However, the error message still not include what value of x could be. You may need to use a format library to create the message:

```c++
    assert(x > 0 && format("x (which is {}) must larger than 0 since ...", x));
```

This looks not that good and is hard to print if you want to have more complicated values (e.g. vector, tuple...).



