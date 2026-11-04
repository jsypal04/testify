#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include "testify.h"


#define TEST_RUN_LOG "/tmp/test_run.log"

struct suite test_suite = {0};

void register_test(struct test test) {
    if (test_suite.tests == NULL) {
        test_suite.len = 0;
        test_suite.cap = 10;
        test_suite.tests = malloc(sizeof(struct test) * test_suite.cap);
    } else if (test_suite.len >= test_suite.cap) {
        test_suite.cap *= 2;
        struct test* tmp = malloc(sizeof(struct test) * test_suite.cap);
        for (size_t i = 0; i < test_suite.len; i++) {
            tmp[i] = test_suite.tests[i];
        }
        free(test_suite.tests);
        test_suite.tests = tmp;
        tmp = NULL;
    }

    test_suite.tests[test_suite.len] = test;
    test_suite.len++;
}

int redirect_stdout() {
    int original_fd = dup(STDOUT_FILENO);
    if (freopen(TEST_RUN_LOG, "a", stdout) == NULL) {
        perror("Failed to redirect stdout\n");
        _exit(EXIT_FAILURE);
    }

    return original_fd;
}

void restore_stdout(int original_fd) {
    fflush(stdout);
    dup2(original_fd, STDOUT_FILENO);
    close(original_fd);
}


void run(struct suite* tests, char* test_label) {
    for (size_t i = 0; i < tests->len; i++) {
        struct test test = tests->tests[i];

        if (test_label != NULL && strcmp(test_label, test.label) != 0)
            continue;

        printf("Running %s:%s... ", test.module, test.label);

        int fd = redirect_stdout();
        printf("BEGIN_TEST:%s\n", test.label);
        test.result = test.run();
        printf("\nEND_TEST\n");
        restore_stdout(fd);

        if (test.result != 0) {
            printf("\033[31mFailed\033[0m\n");
            continue;
        }

        printf("\e[32mPassed\e[0m\n");
    }
}

int testify_run() {
    // Clear the test run log
    FILE* fd = fopen("logs/test_run.log", "w");
    fclose(fd);

    printf("==================\n");
    printf("Running Test Suite\n");
    printf("==================\n\n");

    run(&test_suite, NULL);

    return 0;
}
