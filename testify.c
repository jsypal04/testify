#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/wait.h>

#include "testify.h"


#define TEST_RUN_LOG "/tmp/testify_run.log"

struct output_fds {
    int stdout_fd;
    int stderr_fd;
};

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

struct output_fds redirect_output() {
    fflush(stdout);
    fflush(stderr);
    int original_stdout = dup(STDOUT_FILENO);
    int original_stderr = dup(STDERR_FILENO);
    if (freopen(TEST_RUN_LOG, "a", stdout) == NULL) {
        perror("Failed to redirect stdout\n");
        _exit(EXIT_FAILURE);
    }
    if (freopen(TEST_RUN_LOG, "a", stderr) == NULL) {
        perror("Failed to redirect stderr\n");
        _exit(EXIT_FAILURE);
    }

    struct output_fds fds = {
        .stdout_fd = original_stdout,
        .stderr_fd = original_stderr
    };

    return fds;
}

void restore_output(struct output_fds fds) {
    fflush(stdout);
    fflush(stderr);
    dup2(fds.stdout_fd, STDOUT_FILENO);
    dup2(fds.stderr_fd, STDERR_FILENO);
    close(fds.stdout_fd);
    close(fds.stderr_fd);
}

void restore_captured_output(char* label) {
    printf("\n----- %s: Captured STDIO -----\n", label);

    FILE* output = fopen(TEST_RUN_LOG, "r");
    if (output == NULL) return;

    size_t line_n = 0;
    int32_t read;
    char* line = NULL;
    read = getline(&line, &line_n, output);

    while (read >= 0 && strstr(line, label) == NULL) {
        read = getline(&line, &line_n, output);
    }
    if (read < 0) goto cleanup;

    read = getline(&line, &line_n, output);
    while (read >= 0 && strstr(line, "END_TEST") == NULL) {
        printf("%s", line);
        read = getline(&line, &line_n, output);
    }

    cleanup:
    free(line);
    fclose(output);
}

void run(struct suite* tests, char* module, char* test_label) {
    struct output_fds fds;

    if (tests->tests == NULL) return;

    for (size_t i = 0; i < tests->len; i++) {
        struct test test = tests->tests[i];

        if (module != NULL && strcmp(module, test.module) != 0) {
            tests->tests[i].skipped = 1;
            continue;
        }
        if (test_label != NULL && strcmp(test_label, test.label) != 0) {
            tests->tests[i].skipped = 1;
            continue;
        }

        printf("%s:%s... ", test.module, test.label);
        fds = redirect_output();
        printf("BEGIN_TEST:%s\n", test.label);
        fflush(stdout);
        fflush(stderr);
        restore_output(fds);

        int pid = fork();
        if (pid == 0) {
            fds = redirect_output();
            test.result = test.run();
            exit(test.result);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            test.result = status;
        } else {
            fds = redirect_output();
            printf("\nEND_TEST\n");
            restore_output(fds);
            printf("Failed to fork testing process\n");
            test.skipped = 1;
            tests->tests[i] = test;
            continue;
        }

        fds = redirect_output();
        printf("\nEND_TEST\n");
        restore_output(fds);

        if (test.result != 0) {
            printf("\033[31mFailed\033[0m\n");
            tests->tests[i] = test;
            continue;
        }

        printf("\e[32mPassed\e[0m\n");
        tests->tests[i] = test;
    }
}

int testify_run(int argc, char** argv) {
    // Clear the test run log
    FILE* fd = fopen(TEST_RUN_LOG, "w");
    fclose(fd);

    printf("===== Running Test Suite =====\n\n");

    if (argc == 3) {
        run(&test_suite, argv[1], argv[2]);
    } else if (argc == 2 && strstr(argv[1], ".c") != NULL) {
        run(&test_suite, argv[1], NULL);
    } else if (argc == 2 && strstr(argv[1], ".c") == NULL) {
        run(&test_suite, NULL, argv[1]);
    } else if (argc == 1) {
        run(&test_suite, NULL, NULL);
    } else {
        printf("Usage: ./test-binary [<file>] [<test>]\n");
        return EXIT_FAILURE;
    }

    uint32_t passed  = 0;
    uint32_t failed  = 0;
    uint32_t skipped = 0;

    for (size_t i = 0; i < test_suite.len; i++) {
        struct test test = test_suite.tests[i];
        if (test.skipped) {
            skipped++;
            continue;
        }

        if (test.result == PASSED) {
            passed++;
            continue;
        }

        failed++;
        restore_captured_output(test.label);
    }

    printf("\n===== Test Results =====\n");
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", failed);
    printf("Skipped: %d\n", skipped);
    printf("Total: %zu\n", test_suite.len);

    return failed;
}
