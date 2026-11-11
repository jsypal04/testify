#ifndef TESTS_H
#define TESTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>


#define PASSED   0
#define FAILED   1

#define TEST(name) \
    static int name(void); \
    __attribute__((constructor)) \
    static void __register_##name(void) { \
        struct test __test = { \
            .label  = #name, \
            .module = __FILE__, \
            .run    = name, \
            .result = 0, \
            .skipped = 0 \
        }; \
        register_test(__test); \
    } \
    static int name(void)

#define MAX_LABEL_LEN 256
struct test {
    char label[MAX_LABEL_LEN];
    char module[MAX_LABEL_LEN];
    int (*run)();
    int result;
    uint8_t skipped;
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
