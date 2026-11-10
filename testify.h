#ifndef TESTS_H
#define TESTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>


#define PASSED 0
#define FAILED 1

#define TEST(name) \
    static int name(void); \
    __attribute__((constructor)) \
    static void __register_##name(void) { \
        struct test __test = { \
            .label  = #name, \
            .module = __FILE__, \
            .run    = name, \
            .result = 0 \
        }; \
        register_test(__test); \
    } \
    static int name(void)



struct test {
    char label[256];
    char module[256];
    int (*run)();
    int result;
};

struct suite {
    size_t cap;
    size_t len;
    struct test* tests;
};

void register_test(struct test test);

void run(struct suite* tests, char* module, char* test_label);

int testify_run(int argc, char** argv);

#ifdef __cplusplus
}
#endif

#endif
