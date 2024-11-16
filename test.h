#ifndef KASOUZA_KBUILD_H_TESTS_TEST_H
#define KASOUZA_KBUILD_H_TESTS_TEST_H

typedef enum {
    KTEST_RESULT_OK = 0,
    KTEST_RESULT_FAILED = 1,
} KtestResult;

#define KTEST_FAILF(message, ...) \
    do { \
        printf("\033[31mFAILED [%s, %d]" message "\033[0m\n", __FILE__, __LINE__, __VA_ARGS__); \
        return KTEST_BUILD; \
    } while (0)

#define KTEST_FAIL(message) \
    do { \
        printf("\033[31mFAILED [%s, %d] " message "\033[0m\n", __FILE__, __LINE__); \
        return KTEST_RESULT_FAILED; \
    } while (0)

#define KTEST_ASSERT(test, message) \
    do { \
        if (!test) { \
            KTEST_FAIL(message); \
        } \
    } while (0)

#define KTEST_ASSERTF(test, message, ...) \
    do { \
        if (!test) { \
            KTEST_FAIL(message, __VA_ARGS__); \
        } \
    }


#define KTEST_ASSERT_EQ(a, b, message) \
    do { \
        if ((a) != (b)) { \
            KTEST_FAIL(message); \
        } \
    } while (0)

#define KTEST_ASSERT_EQ_STR(a, b, message) \
    do { \
        if (strcmp(a, b) != 0) { \
            KTEST_FAIL(message); \
        } \
    } while (0)

#define KTEST_ASSERT_EQ_STRF(a, b, message, ...) \
    do { \
        if (strcmp(a, b) != 0) { \
            KTEST_FAILF(message, __VA_ARGS__); \
        } \
    } while (0)

#define KTEST(test_name) \
    do { \
        if (test_name() == KTEST_RESULT_OK) { \
            printf("\033[32mOK [%s, %d] " #test_name "\033[0m\n", __FILE__, __LINE__); \
        } else { \
            return KTEST_RESULT_FAILED; \
        } \
    } while (0)

#endif
