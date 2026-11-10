# Testify

Testify is a super lightweight unit testing framework for C. It was born out
of my frustration at writing unit tests for [Calenter](https://github.com/jsypal04/calenter)
without a good framework.

# Usage

Make sure you have access to the header file `testify.h` and that the
shared object `libtestify.so` is linkable. Alternatively you can just add the
header file and the one C source file to your project and compile and link it
yourself.

To make a test executable, all you need to do is call the `testify_run` function. A
minimal test executable C file would look like this:

```C
int main(int argc, char** argv) {
    return testify_run(argc, argv);
}
```

Each test is a function whose function pointer gets added to the internal test 
suite data structure before the main function is run. To add a test, add the 
following to main executable source file (or another source file linked into the
test executable):

```C
TEST(test_function) {
    /* Execute test */
    
    if (failed) {
        return FAILED;
    }
    return PASSED;
}
```

> [!NOTE]
> It is assumed that all tests have unique names across the entire test suite,
> although this is not enforced. If you do make two tests with the same name
> it will mess with restoration of captured stdio.

Once everything is compiled and linked you just need to run your test executable
to run the entire test suite. You can run only certain files or test by passing
them as command line arguments. For example:

```sh
./test-bin tests/main.c test_function # only runs the test_function test in the file test/main.c

./test-bin test_function # only runs the test named test_function

./test-bin tests/main.c # Runs all tests in tests/main.c

./test-bin # Runs all tests
```
