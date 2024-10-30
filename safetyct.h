#ifndef SAFETYCT_H
#define SAFETYCT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCT_INTERNAL_CONCAT(prefix, suffix) prefix ## suffix
#define CONCAT(prefix, suffix) SCT_INTERNAL_CONCAT(prefix, suffix)

// Generate a unique name with the specified prefix.
#define UNIQUE_NAME(prefix) CONCAT(CONCAT(prefix, __COUNTER__), __LINE__)

#define IS_SAME_TYPE(a, b) __builtin_types_compatible_p(typeof(a), typeof(b))
#define IS_POINTER_OR_ARRAY(x) (__builtin_classify_type(x) == 5)
#define DECAY_POINTER(p) (&*__builtin_choose_expr(IS_POINTER_OR_ARRAY(p), p, NULL))
#define IS_POINTER(x) IS_SAME_TYPE(x, DECAY_POINTER(x))
#define IS_COMPTIME_KNOWN(x) __builtin_constant_p(x)

#define IS_EQUAL(a, b)                                  \
    ({                                                  \
        GCC_DIAGNOSTIC_IGNORED("-Wint-conversion")      \
        GCC_DIAGNOSTIC_IGNORED("-Wnonnull")             \
        _Generic((b),                                   \
            char*: !strcmp(a, b),                       \
            default: (a) == (b)                         \
        );                                              \
        GCC_DIAGNOSTIC_WARNING("-Wint-conversion")      \
        GCC_DIAGNOSTIC_WARNING("-Wnonnull")             \
    })

// Get the length of an array.
// Crash if used on a pointer.
#define ARRAY_LENGTH(array)                                     \
    ({                                                          \
        if (IS_POINTER(array)) {                                \
            SCT_INTERNAL_CRASH("ARRAY_LENGTH", array);          \
        }                                                       \
        (unsigned long long)(sizeof(array) / sizeof(*(array))); \
    })

//
// FUNCTION ATTRIBUTES
//

// Define a function with the const attribute.
// The function should be a pure function.
#define CONSTFUNC __attribute__((const))

#define INITFUNC __attribute__((constructor(101 + __COUNTER__)))

// Define a function that runs before main.
// Useful for initializing global variables.
#define INIT INITFUNC void UNIQUE_NAME(init)(void)

//
// MACRO UTILS
//

#define TO_STRING(x) # x
#define TO_ARRAY(args...) ((void*[]){args})

#define GCC_DIAGNOSTIC_IGNORED(x) _Pragma(TO_STRING(GCC diagnostic ignored x))
#define GCC_DIAGNOSTIC_WARNING(x) _Pragma(TO_STRING(GCC diagnostic warning x))

#define ARGS_COUNT(args...)                         \
    ({                                              \
        GCC_DIAGNOSTIC_IGNORED("-Wint-conversion")  \
        (sizeof((void*[]){args}) / sizeof(void*));  \
        GCC_DIAGNOSTIC_WARNING("-Wint-conversion")  \
    })

#define ARGS_GET(index, args...)                    \
    ({                                              \
        GCC_DIAGNOSTIC_IGNORED("-Wint-conversion")  \
        (((void*[]){args})[(index)]);               \
        GCC_DIAGNOSTIC_WARNING("-Wint-conversion")  \
    })

#define ARGS_HAS_INDEX(index, args...)          \
    ({                                          \
        GCC_DIAGNOSTIC_IGNORED("-Wtype-limits") \
        (ARGS_COUNT(args) - 1 >= (index));      \
        GCC_DIAGNOSTIC_WARNING("-Wtype-limits") \
    })

#define ARGS_GET_OR_DEFAULT(index, default, args...)    \
    ({                                                  \
        GCC_DIAGNOSTIC_IGNORED("-Warray-bounds")        \
        typeof(default) value = default;                \
        if (ARGS_HAS_INDEX(index, args)) {              \
            GCC_DIAGNOSTIC_IGNORED("-Wint-conversion")  \
            value = ARGS_GET(index, args);              \
            GCC_DIAGNOSTIC_WARNING("-Wint-conversion")  \
        }                                               \
        value;                                          \
        GCC_DIAGNOSTIC_WARNING("-Warray-bounds")        \
    })

//
// POINTER OPERATIONS
//

// Allocate memory for a new type with calloc.
// The optional count specifies the number of elements.
// Crash if calloc fails.
#define NEW(type, count...)                             \
    ({                                                  \
        GCC_DIAGNOSTIC_IGNORED("-Wint-conversion")      \
        int n = ARGS_GET_OR_DEFAULT(0, 1, count);       \
        void *pointer = calloc(n, sizeof(type));        \
        if (pointer == NULL) {                          \
            SCT_INTERNAL_CRASH("NEW", TO_STRING(type)); \
        }                                               \
        pointer;                                        \
        GCC_DIAGNOSTIC_WARNING("-Wint-conversion")      \
    })

// Resize heap allocation with realloc.
// N is the number of elements desired.
// Crash if pointer is NULL.
#define RESIZE(pointer, count)                                  \
    ({                                                          \
        if ((pointer) == NULL) {                                \
            SCT_INTERNAL_CRASH("RESIZE", TO_STRING(pointer))    \
        }                                                       \
        pointer = realloc((pointer), sizeof(*pointer) * count); \
        if ((pointer) == NULL) {                                \
            SCT_INTERNAL_CRASH("RESIZE", TO_STRING(pointer))    \
        }                                                       \
        pointer;                                                \
    })

// Set the field of a heap allocated struct.
// Crash if pointer is NULL.
#define SET_FIELD(pointer, field, value)                            \
    do {                                                            \
        if ((pointer) == NULL) {                                    \
            SCT_INTERNAL_CRASH("SET_FIELD", TO_STRING(pointer));    \
        }                                                           \
        (pointer)->field = (value);                                 \
    } while (0)

// Get the field of a heap allocated struct.
// Crash if pointer is NULL.
#define GET_FIELD(pointer, field)                                   \
    ({                                                              \
        if ((pointer) == NULL) {                                    \
            SCT_INTERNAL_CRASH("GET_FIELD", TO_STRING(pointer));    \
        }                                                           \
        (pointer)->field;                                           \
    })

// Set the index of a pointer.
// Crash if pointer is NULL.
#define SET_INDEX(pointer, index, value)                                    \
    do {                                                                    \
        if ((pointer) == NULL) {                                            \
            SCT_INTERNAL_CRASH("SET_INDEX", TO_STRING(pointer));            \
        }                                                                   \
        (pointer)[index] = (value);                                         \
    } while (0)

// Get the index of a pointer.
// Crash if pointer is NULL.
#define GET_INDEX(pointer, index)                                           \
    ({                                                                      \
        if ((pointer) == NULL) {                                            \
            SCT_INTERNAL_CRASH("GET_INDEX", TO_STRING(pointer));            \
        }                                                                   \
        (((pointer)[index]);                                                \
    })

// Zero out every field of a heap allocated struct.
// Crash if pointer is NULL.
#define CLEAR(pointer)                                          \
    do {                                                        \
        if ((pointer) == NULL) {                                \
            SCT_INTERNAL_CRASH("CLEAR", TO_STRING(pointer));    \
        }                                                       \
        *(pointer) = (typeof(*(pointer))) {0};                  \
    } while(0)

// Free a pointer and assign NULL to it.
#define RELEASE(pointer)    \
    do {                    \
        free(pointer);      \
        (pointer) = NULL;   \
    } while (0)

#define SCT_INTERNAL_TRACEBACK_LEADING_TEXT "Traceback (most recent call last):\n"
#define SCT_INTERNAL_TRACEBACK_ERROR_FORMAT "    File %s, line %d, in function %s\n        %s %s\n"

//
//  DEBUG MODE: Expand debugging capabilities with longer error messages
//

#ifdef DEBUG

    #define SCT_INTERNAL_TRACEBACK_COUNT_MAX 128
    #define SCT_INTERNAL_TRACEBACK_LENGTH_MAX 256

    static int sct_internal_traceback_count __attribute__ ((unused)) = 0;
    static char sct_internal_traceback[SCT_INTERNAL_TRACEBACK_COUNT_MAX][SCT_INTERNAL_TRACEBACK_LENGTH_MAX] __attribute__ ((unused));

    #define SCT_INTERNAL_TRACEBACK_RESET sct_internal_traceback_count = 0;

    #define SCT_INTERNAL_TRACEBACK_PUSH(file, line, func, cause, error)             \
        if (sct_internal_traceback_count < SCT_INTERNAL_TRACEBACK_COUNT_MAX) {      \
            snprintf(                                                               \
                (char*) &sct_internal_traceback[sct_internal_traceback_count++],    \
                SCT_INTERNAL_TRACEBACK_LENGTH_MAX,                                  \
                SCT_INTERNAL_TRACEBACK_ERROR_FORMAT,                                \
                file, line, func, cause, # error                                    \
            );                                                                      \
        }

    #define SCT_INTERNAL_TRACEBACK_PRINT(file, line, func, cause, error)            \
        do {                                                                        \
            fprintf(stderr, SCT_INTERNAL_TRACEBACK_LEADING_TEXT);                   \
            for (int i = 0; i < sct_internal_traceback_count; i += 1)               \
                fprintf(stderr, "%s", sct_internal_traceback[i]);                   \
        } while (0);

#else

    #define SCT_INTERNAL_TRACEBACK_RESET
    #define SCT_INTERNAL_TRACEBACK_PUSH(file, line, func, cause, error)
    #define SCT_INTERNAL_TRACEBACK_PRINT(file, line, func, cause, error)    \
        fprintf(                                                            \
            stderr,                                                         \
            SCT_INTERNAL_TRACEBACK_LEADING_TEXT                             \
            SCT_INTERNAL_TRACEBACK_ERROR_FORMAT,                            \
            file, line, func, cause, # error                                \
        );                                                                  \

#endif

//
//  TESTING MODE: Run tests defined in the source files
//

#ifdef TESTS

    #define SCT_INTERNAL_TEST_MESSAGES_COUNT_MAX 1024
    #define SCT_INTERNAL_TEST_MESSAGES_LENGTH_MAX 256

    static char sct_internal_test_messages[SCT_INTERNAL_TEST_MESSAGES_COUNT_MAX][SCT_INTERNAL_TEST_MESSAGES_LENGTH_MAX] __attribute__ ((unused));
    static unsigned sct_internal_test_count __attribute__ ((unused)) = 0;
    static unsigned sct_internal_test_fail_count __attribute__ ((unused)) = 0;

    #define ASSERT_EQUAL(a, b)                                                          \
        do {                                                                            \
            typeof(a) evaluated = a;                                                    \
            if (!IS_EQUAL(evaluated, b)) {                                              \
                SCT_INTERNAL_TEST_MESSAGES_PUSH(__FILE__, __LINE__, a, b, evaluated);   \
                return 1;                                                               \
            }                                                                           \
        } while (0)

    #define ASSERT_NOT_EQUAL(a, b)                                                      \
        do {                                                                            \
            typeof(a) evaluated = a;                                                    \
            if (IS_EQUAL(evaluated, b)) {                                               \
                SCT_INTERNAL_TEST_MESSAGES_PUSH(__FILE__, __LINE__, a, b, evaluated);   \
                return 1;                                                               \
            }                                                                           \
        } while (0)

    #define ASSERT_NONE(expr) ASSERT_EQUAL(expr, 0)
    #define ASSERT_SOME(expr) ASSERT_NOT_EQUAL(expr, 0)

    #define SCT_INTERNAL_TEST_FAILURE_BASE_FORMAT "\e[31mFailed test:\e[0m %s (%s:%d)\n   \e[33mExpected:\e[0m %s = %s\n   \e[33mReceived:\e[0m %s = "

    #define SCT_INTERNAL_RESOLVE_TEST_FAILURE_FORMAT(x)                             \
        _Generic(x,                                                                 \
            char: SCT_INTERNAL_TEST_FAILURE_BASE_FORMAT "%c\n",                     \
            unsigned char: SCT_INTERNAL_TEST_FAILURE_BASE_FORMAT "%d\n",            \
            int: SCT_INTERNAL_TEST_FAILURE_BASE_FORMAT "%d\n",                      \
            long int: SCT_INTERNAL_TEST_FAILURE_BASE_FORMAT "%ld\n",                \
            long long: SCT_INTERNAL_TEST_FAILURE_BASE_FORMAT "%lld\n",              \
            unsigned: SCT_INTERNAL_TEST_FAILURE_BASE_FORMAT "%u\n",                 \
            unsigned long long: SCT_INTERNAL_TEST_FAILURE_BASE_FORMAT "%llu\n",     \
            char*: SCT_INTERNAL_TEST_FAILURE_BASE_FORMAT "\"%s\"\n"                 \
        )

    #define SCT_INTERNAL_TEST_MESSAGES_PUSH(file, line, expr, value, eval)                      \
        if (sct_internal_test_fail_count < SCT_INTERNAL_TEST_MESSAGES_COUNT_MAX) {              \
            snprintf(                                                                           \
                (char*) &sct_internal_test_messages[sct_internal_test_fail_count++],            \
                SCT_INTERNAL_TEST_MESSAGES_LENGTH_MAX,                                          \
                SCT_INTERNAL_RESOLVE_TEST_FAILURE_FORMAT(eval),                                 \
                __desc, file, line, TO_STRING(expr), TO_STRING(value), TO_STRING(expr), eval    \
            );                                                                                  \
        }

    #define TEST(description, body)                                                         \
        __attribute__((constructor(200 + __COUNTER__))) void UNIQUE_NAME(sct_test_)(void) { \
            const char *__desc __attribute__ ((unused)) = description;                      \
            sct_internal_test_count += 1;                                                   \
            int run_test(void) {                                                            \
                body;                                                                       \
                return 0;                                                                   \
            }                                                                               \
            int error = run_test();                                                         \
            char *status = "PASS", *color = "\e[32m";                                       \
            if (error) {                                                                    \
                status = "FAIL";                                                            \
                color = "\e[31m";                                                           \
            }                                                                               \
            printf(                                                                         \
                "%s[%s]\e[0m %s (%s:%d)\n",                                                 \
                color, status, description, __FILE__, __LINE__                              \
            );                                                                              \
        }

    // TODO: If used in more than one compilation units, this function will run multiple times
    static __attribute__ ((constructor(65535))) void sct_internal_run_after_tests(void) {
        if (sct_internal_test_count == 0) {
            return;
        }
        unsigned test_pass_count = sct_internal_test_count - sct_internal_test_fail_count;
        for (unsigned i = 0; i < sct_internal_test_fail_count; i += 1) {
            printf("\n%s", sct_internal_test_messages[i]);
        }
        printf(
            "\n\e[34mTotal:\e[0m %u, \e[32mPass:\e[0m %u, \e[31mFail:\e[0m %u\n",
            sct_internal_test_count, test_pass_count, sct_internal_test_fail_count
        );
        exit(sct_internal_test_fail_count > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
    }

#else

    #define TEST(description, body)
    #define ASSERT_EQUAL(a, b) PRESUME((a) == (b))
    #define ASSERT_NOT_EQUAL(a, b) PRESUME((a) != (b))
    #define ASSERT_NONE(error) PRESUME((error) == 0)
    #define ASSERT_SOME(error) PRESUME((error) != 0)

#endif

//
//  THROW - CRASH
//

#define SCT_INTERNAL_THROW(cause, error)                                                    \
    do {                                                                                    \
        SCT_INTERNAL_TRACEBACK_PUSH(__FILE__, __LINE__, __PRETTY_FUNCTION__, cause, error)  \
        return error;                                                                       \
    } while (0)

#define THROW_IF(condition, error) do { if (condition) { SCT_INTERNAL_THROW("THROW_IF", error); } } while (0)
#define THROW_UNLESS(condition, error) do { if (!(condition)) { SCT_INTERNAL_THROW("THROW_UNLESS", error); } } while (0)
#define THROW(reason) SCT_INTERNAL_THROW("THROW", reason)

#define SCT_INTERNAL_CRASH(cause, error)                                                        \
    do {                                                                                        \
        SCT_INTERNAL_TRACEBACK_PUSH(__FILE__, __LINE__, __PRETTY_FUNCTION__, cause, error)      \
        SCT_INTERNAL_TRACEBACK_PRINT(__FILE__, __LINE__, __PRETTY_FUNCTION__, cause, error)     \
        exit(_Generic(error, char*: 1, default: error));                                        \
    } while (0)

#define CRASH_IF(condition) do { if (condition) { SCT_INTERNAL_CRASH("CRASH_IF", condition); } } while (0)
#define CRASH_UNLESS(condition) do { if (!(condition)) { SCT_INTERNAL_CRASH("CRASH_UNLESS", !(condition)); } } while (0)
#define CRASH(reason) SCT_INTERNAL_CRASH("CRASH", reason)

// Try to successfully run a SafetyCT function.
// Throw any error that occurs, continue if there is no error.
// This can be used if it's not important to handle every error individually.
#define THROW_SOME(error)                               \
    do {                                                \
        typeof(error) evaluated = error;                \
        if (evaluated != 0) {                           \
            SCT_INTERNAL_THROW("THROW_SOME", error);    \
        }                                               \
    } while (0)

#define THROW_SOME_AS(error_src, error_dst)                 \
    do {                                                    \
        typeof(error) evaluated = error_src;                \
        if (evaluated != 0) {                               \
            SCT_INTERNAL_THROW("THROW_SOME_AS", error_dst); \
        }                                                   \
    } while (0)

#define CRASH_SOME(error)                               \
    do {                                                \
        typeof(error) evaluated = error;                \
        if (evaluated != 0) {                           \
            SCT_INTERNAL_CRASH("CRASH_SOME", error);    \
        }                                               \
    } while (0)

//
//  SWITCH - CASE - THROW - CRASH
//

// Propagate an error to the calling function instead of handling it yourself.
#define THROW_CASE(x) case x: SCT_INTERNAL_THROW("THROW_CASE", x)

// Propagate an error as a different error to the calling function instead of handling it yourself.
#define THROW_CASE_AS(x, y) case x: SCT_INTERNAL_THROW("THROW_CASE_AS", y)

// Refuse to deal with the error and crash the program.
// Call this in an error-handler `switch` in situations where the error should never occur.
// Only use this if you know that the error should never be returned!
#define CRASH_CASE(x) case x: SCT_INTERNAL_CRASH("CRASH_CASE", x)

//
//  DEFER
//

#define SCT_INTERNAL_DEFER(condition, statement, cleanup_var_name, cleanup_func_name)   \
    void cleanup_func_name(void *arg) {                                                 \
        (void) arg;                                                                     \
        if (condition) {                                                                \
            statement;                                                                  \
        }                                                                               \
    }                                                                                   \
    int cleanup_var_name __attribute__ ((__cleanup__(cleanup_func_name))) = 0

// Defer running statements until the end of the current scope if the condition is truthy.
// If there are multiple defers in the same scope, they will be called in reverse order.
#define DEFER_IF(condition, statement) SCT_INTERNAL_DEFER(condition, statement, UNIQUE_NAME(cleanup_var),  UNIQUE_NAME(cleanup_func))

// Defer running statements until the end of the current scope.
// If there are multiple defers in the same scope, they will be called in reverse order.
#define DEFER(statement) SCT_INTERNAL_DEFER(1, statement, UNIQUE_NAME(cleanup_var),  UNIQUE_NAME(cleanup_func))

#endif
