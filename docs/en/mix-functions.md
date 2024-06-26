Mix Assertion, Logging, Unit Testing and Fuzzing 
=================================================

## Issues in Unit Tesing

Almost every project needs unit testing. This is most important way to build a strong software but it sometimes may be hard to check boundary cases. Let's consider an example:

```c++
Object* create(int param) 
{
    Object* obj = NULL;
    // try create using method A
    // do sth.
    obj = ...;
    if (obj) {
        return obj
    }

    // try create using method B
    // do sth.
    obj = ...;
    if (obj) {
        return obj;
    }
    return obj;
}
```

The function `create` try to create object using different ways, if one is failed and then try another. But how should we write test cases to check it. You didn't know this is from method A or B and the output looks the same.

A good news is we can add log. This is a way to know some information for the function we just ran:

```c++
Object* create(int param) 
{
    Object* obj = NULL;
    // try create using method A
    // do sth.
    obj = ...;
    if (obj) {
        LOG(INFO) << "create object using method A.";
        return obj
    }

    // try create using method B
    // do sth.
    obj = ...;
    if (obj) {
        LOG(INFO) << "create object using method B.";
        return obj;
    }
    return obj;
}
```

After running the function, we can see the output to know where it came from. However, there is no way to check the log in the unit testing. 
Yes, maybe you could read the file and see what output it wrote to the file but it's very complicated and not realiable.

I am thinking, maybe we could let the log data become **accessable**.

## Structure Logging

There is way to log the data, 




## Another problem with assertion

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


