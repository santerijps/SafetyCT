#ifndef SAFETYCT_H
#define SAFETYCT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
//  PRINTING
//

#define ERRORF(format, args...) fprintf(stderr, format, ## args)
#define PANICF(format, args...) do { ERRORF(format, ## args); exit(1); } while (0)

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
            char*: !strcmp((a), (b)),                   \
            default: (a) == (b)                         \
        );                                              \
        GCC_DIAGNOSTIC_WARNING("-Wint-conversion")      \
        GCC_DIAGNOSTIC_WARNING("-Wnonnull")             \
    })

#define CAST(value, type) ((type)(value))
#define ARRAY_LENGTH(array) ((unsigned long long)(sizeof(array) / sizeof(*(array))))

//
// FUNCTION ATTRIBUTES
//

#define CONSTFUNC __attribute__((const, warn_unused_result))
#define INITFUNC __attribute__((constructor(101 + __COUNTER__)))
#define REQUIRE_RESULT __attribute__((warn_unused_result))
#define INIT INITFUNC void UNIQUE_NAME(init)(void)

#define TO_STRING(x) # x
#define TO_ARRAY(args...) ((void*[]){args})

//
//  GGC DIAGNOSTIC PRAGMAS
//

#define GCC_DIAGNOSTIC_IGNORED(x) _Pragma(TO_STRING(GCC diagnostic ignored x))
#define GCC_DIAGNOSTIC_WARNING(x) _Pragma(TO_STRING(GCC diagnostic warning x))

//
//  VA ARGS
//

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
//  TRACEBACK
//

#define SCT_INTERNAL_TRACEBACK_LEADING_TEXT     \
    "Traceback (most recent call last):\n"

#define SCT_INTERNAL_TRACEBACK_ERROR_FORMAT     \
    "    File %s, line %d, in function %s\n"    \
    "        %s %s"

#define SCT_INTERNAL_TRACEBACK_ERRORF_FORMAT    \
    "    File %s, line %d, in function %s\n"    \
    "        %s"

#define SCT_INTERNAL_TRACEBACK_RESOLVE_FORMAT(evaluation)                       \
    _Generic((evaluation),                                                      \
        char: SCT_INTERNAL_TRACEBACK_ERROR_FORMAT " => %c\n",                   \
        unsigned char: SCT_INTERNAL_TRACEBACK_ERROR_FORMAT " => %d\n",          \
        int: SCT_INTERNAL_TRACEBACK_ERROR_FORMAT " => %d\n",                    \
        long int: SCT_INTERNAL_TRACEBACK_ERROR_FORMAT " => %ld\n",              \
        long long: SCT_INTERNAL_TRACEBACK_ERROR_FORMAT " => %lld\n",            \
        unsigned: SCT_INTERNAL_TRACEBACK_ERROR_FORMAT " => %u\n",               \
        unsigned long long: SCT_INTERNAL_TRACEBACK_ERROR_FORMAT " => %llu\n",   \
        char*: SCT_INTERNAL_TRACEBACK_ERROR_FORMAT " => \"%s\"\n",              \
        default: SCT_INTERNAL_TRACEBACK_ERROR_FORMAT " => %p\n"                 \
    )

#define SCT_INTERNAL_TRACEBACK_RESOLVE_FORMAT_WITH_ERROR(evaluation)                \
    _Generic((evaluation),                                                          \
        char: SCT_INTERNAL_TRACEBACK_ERROR_FORMAT " => %c -> %s\n",                 \
        unsigned char: SCT_INTERNAL_TRACEBACK_ERROR_FORMAT " => %d -> %s\n",        \
        int: SCT_INTERNAL_TRACEBACK_ERROR_FORMAT " => %d -> %s\n",                  \
        long int: SCT_INTERNAL_TRACEBACK_ERROR_FORMAT " => %ld -> %s\n",            \
        long long: SCT_INTERNAL_TRACEBACK_ERROR_FORMAT " => %lld -> %s\n",          \
        unsigned: SCT_INTERNAL_TRACEBACK_ERROR_FORMAT " => %u -> %s\n",             \
        unsigned long long: SCT_INTERNAL_TRACEBACK_ERROR_FORMAT " => %llu -> %s\n", \
        char*: SCT_INTERNAL_TRACEBACK_ERROR_FORMAT " => \"%s\" -> %s\n",            \
        default: SCT_INTERNAL_TRACEBACK_ERROR_FORMAT " => %p -> %s\n"               \
    )

//
//  DEBUG MODE: Expand debugging capabilities with longer error messages
//

#ifdef DEBUG

    //
    //  TRACEBACK
    //

    #define TRACE(expression)                                               \
        ({                                                                  \
            typeof(expression) evaluation = (expression);                   \
            SCT_INTERNAL_TRACEBACK_PUSH("TRACE", expression, evaluation);   \
            evaluation;                                                     \
        })

    #define SCT_INTERNAL_TRACEBACK_COUNT_MAX 256
    #define SCT_INTERNAL_TRACEBACK_LENGTH_MAX 256

    static int sct_internal_traceback_count __attribute__ ((unused)) = 0;
    static char sct_internal_traceback[SCT_INTERNAL_TRACEBACK_COUNT_MAX][SCT_INTERNAL_TRACEBACK_LENGTH_MAX] __attribute__ ((unused));

    #define SCT_INTERNAL_TRACEBACK_RESET sct_internal_traceback_count = 0;

    // TODO: Else crash? At least warn that the traceback count has been reached
    #define SCT_INTENRAL_TRACEBACK_PUSH_FORMAT(format, args...)                         \
        do {                                                                            \
            if (sct_internal_traceback_count < SCT_INTERNAL_TRACEBACK_COUNT_MAX) {      \
                snprintf(                                                               \
                    (char*) &sct_internal_traceback[sct_internal_traceback_count++],    \
                    SCT_INTERNAL_TRACEBACK_LENGTH_MAX,                                  \
                    format,                                                             \
                    ## args                                                             \
                );                                                                      \
            }                                                                           \
        } while (0);

    #define SCT_INTERNAL_TRACEBACK_PUSH(description, expression, evaluation)                        \
        SCT_INTENRAL_TRACEBACK_PUSH_FORMAT(                                                         \
            SCT_INTERNAL_TRACEBACK_RESOLVE_FORMAT(evaluation),                                      \
            __FILE__, __LINE__, __PRETTY_FUNCTION__, description, TO_STRING(expression), evaluation \
        )

    #define SCT_INTERNAL_TRACEBACK_PUSH_WITH_ERROR(description, expression, evaluation, error)                          \
        SCT_INTENRAL_TRACEBACK_PUSH_FORMAT(                                                                             \
            SCT_INTERNAL_TRACEBACK_RESOLVE_FORMAT_WITH_ERROR(evaluation),                                               \
            __FILE__, __LINE__, __PRETTY_FUNCTION__, description, TO_STRING(expression), evaluation, TO_STRING(error)   \
        )

    #define SCT_INTERNAL_TRACEBACK_PUSHF(description, format, args...)          \
        SCT_INTENRAL_TRACEBACK_PUSH_FORMAT(                                     \
            SCT_INTERNAL_TRACEBACK_ERRORF_FORMAT "\n        " format,           \
            __FILE__, __LINE__, __PRETTY_FUNCTION__, description, ## args       \
        )

    #define SCT_INTERNAL_TRACEBACK_PUSHF_WITH_ERROR(description, error, format, args...)    \
        SCT_INTENRAL_TRACEBACK_PUSH_FORMAT(                                                 \
            SCT_INTERNAL_TRACEBACK_ERRORF_FORMAT "-> %s\n        " format,                  \
            __FILE__, __LINE__, __PRETTY_FUNCTION__, description, # error, ## args          \
        )

    #define SCT_INTERNAL_TRACEBACK_PRINT(description, expression, evaluation)   \
        do {                                                                    \
            fprintf(stderr, SCT_INTERNAL_TRACEBACK_LEADING_TEXT);               \
            for (int i = 0; i < sct_internal_traceback_count; i += 1)           \
                fprintf(stderr, "%s", sct_internal_traceback[i]);               \
        } while (0);

    #define SCT_INTERNAL_TRACEBACK_PRINTF(description, format, args...) SCT_INTERNAL_TRACEBACK_PRINT(0, 0, 0)

    //
    //  INTERNAL MEMORY ALLOCATION
    //

    // Must not be divisible by 8, sizeof(void*)
    #define SCTI_ALLOC_SIZE 1019
    #define SCTI_ALLOC_INFO_SIZE 64
    #define SCTI_ALLOC_HASH(pointer) (((size_t)pointer) % SCTI_ALLOC_SIZE)

    static void const* scti_alloc_pointers[SCTI_ALLOC_SIZE];
    static size_t scti_alloc_counts[SCTI_ALLOC_SIZE];                           // element count
    static size_t scti_alloc_sizes[SCTI_ALLOC_SIZE];                            // element size
    static char scti_alloc_infos[SCTI_ALLOC_SIZE][SCTI_ALLOC_INFO_SIZE];
    static size_t scti_alloc_count = 0;
    static int scti_alloc_destruct = 1;

    #define SCTI_ALLOC_SET_INDEX(pointer, count, size)      \
        do {                                                \
            size_t const hash = SCTI_ALLOC_HASH(pointer);   \
            if (scti_alloc_pointers[hash] != 0) {           \
                scti_alloc_destruct = 0;                    \
                SCT_INTERNAL_CRASHF(                        \
                    "ALLOC_SET_INDEX",                      \
                    "Hash collision!\n"                     \
                );                                          \
            }                                               \
            scti_alloc_pointers[hash] = pointer;            \
            scti_alloc_counts[hash] = count;                \
            scti_alloc_sizes[hash] = size;                  \
            sprintf(                                        \
                scti_alloc_infos[hash],                     \
                "file %s, line %d, in function %s",         \
                __FILE__, __LINE__, __PRETTY_FUNCTION__     \
            );                                              \
            scti_alloc_count += 1;                          \
        } while (0);

    #define SCTI_ALLOC_UNSET_INDEX(pointer)                             \
        do {                                                            \
            size_t const hash = SCTI_ALLOC_HASH(pointer);               \
            if (scti_alloc_pointers[hash] == 0) {                       \
                scti_alloc_destruct = 0;                                \
                SCT_INTERNAL_CRASHF(                                    \
                    "ALLOC_UNSET_INDEX",                                \
                    "Hash not found!\n"                                 \
                );                                                      \
            }                                                           \
            scti_alloc_pointers[hash] = 0;                              \
            scti_alloc_counts[hash] = 0;                                \
            scti_alloc_sizes[hash] = 0;                                 \
            memset(scti_alloc_infos[hash], 0, SCTI_ALLOC_INFO_SIZE);    \
            scti_alloc_count -= 1;                                      \
        } while (0);

    #define SCTI_ALLOC_BOUNDS_CHECK(pointer, index)             \
        do {                                                    \
            size_t const hash = SCTI_ALLOC_HASH(pointer);       \
            if (scti_alloc_pointers[hash] == 0) {               \
                break;                                          \
            }                                                   \
            if (index >= scti_alloc_counts[hash]) {             \
                scti_alloc_destruct = 0;                        \
                SCT_INTERNAL_CRASHF(                            \
                    "ALLOC_BOUNDS_CHECK",                       \
                    "Index out of bounds: %llu > %llu\n",       \
                    (size_t)index, scti_alloc_counts[hash] - 1  \
                );                                              \
            }                                                   \
        } while (0);

    __attribute__((destructor))
    static inline void scti_check_for_leaks(void) {
        void const* pointer;
        size_t count, size;
        char const* info;

        if (!scti_alloc_destruct || scti_alloc_count == 0) {
            return;
        }

        fprintf(stderr, "\e[31m\nMemory leaks:\n");

        for (int i = 0; i < SCTI_ALLOC_SIZE; i += 1) {
            pointer = scti_alloc_pointers[i];

            if (pointer != 0) {
                count = scti_alloc_counts[i];
                size = scti_alloc_sizes[i];
                info = scti_alloc_infos[i];
                fprintf(stderr, "  %10llu bytes allocated in %s\n", count * size, info);
            }
        }
    }

#else

    //
    //  TRACEBACK
    //

    #define TRACE(expression) expression
    #define SCT_INTERNAL_TRACEBACK_RESET
    #define SCT_INTERNAL_TRACEBACK_PUSH(description, expression, evaluation)
    #define SCT_INTERNAL_TRACEBACK_PUSH_WITH_ERROR(description, expression, evaluation, error)
    #define SCT_INTERNAL_TRACEBACK_PUSHF(description, format, args...)
    #define SCT_INTERNAL_TRACEBACK_PUSHF_WITH_ERROR(description, error, format, args...)

    #define SCT_INTERNAL_TRACEBACK_PRINT(description, expression, evaluation)                               \
        do {                                                                                                \
            fprintf(stderr, SCT_INTERNAL_TRACEBACK_LEADING_TEXT);                                           \
            fprintf(                                                                                        \
                stderr,                                                                                     \
                SCT_INTERNAL_TRACEBACK_RESOLVE_FORMAT(evaluation),                                          \
                __FILE__, __LINE__, __PRETTY_FUNCTION__, description, TO_STRING(expression), evaluation     \
            );                                                                                              \
        } while (0);

    #define SCT_INTERNAL_TRACEBACK_PRINTF(description, format, args...) \
        do {                                                            \
            fprintf(stderr, SCT_INTERNAL_TRACEBACK_LEADING_TEXT);       \
            fprintf(                                                    \
                stderr,                                                 \
                SCT_INTERNAL_TRACEBACK_ERRORF_FORMAT "\n"               \
                "        "                                              \
                format,                                                 \
                __FILE__, __LINE__, __PRETTY_FUNCTION__, description,   \
                ## args                                                 \
            );                                                          \
        } while (0);

    //
    //  INTERNAL MEMORY ALLOCATION
    //

    #define SCTI_ALLOC_SET_INDEX(pointer, count, size)
    #define SCTI_ALLOC_UNSET_INDEX(pointer)
    #define SCTI_ALLOC_BOUNDS_CHECK(pointer, index)

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

    #define ASSERT_NOT_EQUAL(a, b)                                  \
        do {                                                        \
            typeof(a) evaluated = a;                                \
            if (IS_EQUAL(evaluated, b)) {                           \
                SCT_INTERNAL_TEST_MESSAGES_PUSH(a, b, evaluated);   \
                return 1;                                           \
            }                                                       \
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

    #define SCT_INTERNAL_TEST_MESSAGES_PUSH(expr, value, eval)                                          \
        if (sct_internal_test_fail_count < SCT_INTERNAL_TEST_MESSAGES_COUNT_MAX) {                      \
            snprintf(                                                                                   \
                (char*) &sct_internal_test_messages[sct_internal_test_fail_count++],                    \
                SCT_INTERNAL_TEST_MESSAGES_LENGTH_MAX,                                                  \
                SCT_INTERNAL_RESOLVE_TEST_FAILURE_FORMAT(eval),                                         \
                __desc, __FILE__, __LINE__, TO_STRING(expr), TO_STRING(value), TO_STRING(expr), eval    \
            );                                                                                          \
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
    #define ASSERT_EQUAL(a, b) CRASH_IF((a) != (b))
    #define ASSERT_NOT_EQUAL(a, b) CRASH_IF((a) == (b))
    #define ASSERT_NONE(error) CRASH_IF((error) != 0)
    #define ASSERT_SOME(error) CRASH_IF((error) == 0)

#endif

//
//  MEMORY ALLOCATION
//

#define MALLOC(count, type)                                     \
    ({                                                          \
        void* const pointer = malloc(sizeof(type) * (count));   \
        SCTI_ALLOC_SET_INDEX(pointer, count, sizeof(type))      \
        pointer;                                                \
    })

#define CALLOC(count, type)                                 \
    ({                                                      \
        void* const pointer = calloc(count, sizeof(type));  \
        SCTI_ALLOC_SET_INDEX(pointer, count, sizeof(type))  \
        pointer;                                            \
    })

#define REALLOC(pointer, count, type)                                       \
    ({                                                                      \
        SCTI_ALLOC_UNSET_INDEX(pointer)                                     \
        void* const new_pointer = realloc(pointer, sizeof(type) * (count)); \
        SCTI_ALLOC_SET_INDEX(new_pointer, count, sizeof(type))              \
        new_pointer;                                                        \
    })

#define FREE(pointer)                       \
    do {                                    \
        SCTI_ALLOC_UNSET_INDEX(pointer)     \
        free(pointer);                      \
    } while (0)

#define DEREF(pointer, index)                       \
    *({                                             \
        SCTI_ALLOC_BOUNDS_CHECK((pointer), (index)) \
        (pointer) + (index);                        \
    })

//
//  THROW
//

#define SCT_INTERNAL_THROW(description, expression, evaluation)             \
    do {                                                                    \
        SCT_INTERNAL_TRACEBACK_PUSH(description, expression, evaluation)    \
        return evaluation;                                                  \
    } while (0)

#define SCT_INTERNAL_THROW_AS(description, expression, evaluation, error)                   \
    do {                                                                                    \
        SCT_INTERNAL_TRACEBACK_PUSH_WITH_ERROR(description, expression, evaluation, error)  \
        return error;                                                                       \
    } while (0)

#define THROW(error)                                    \
    do {                                                \
        typeof(error) evaluation = (error);             \
        SCT_INTERNAL_THROW("THROW", error, evaluation); \
    } while (0)

#define THROW_IF(condition, error)                                          \
    do {                                                                    \
        if (condition) {                                                    \
            SCT_INTERNAL_THROW_AS("THROW_IF", condition, error, error);     \
        }                                                                   \
        SCT_INTERNAL_TRACEBACK_RESET                                        \
    } while (0)

#define THROW_SOME(expression)                                          \
    do {                                                                \
        typeof(expression) evaluation = (expression);                   \
        if (evaluation != 0) {                                          \
            SCT_INTERNAL_THROW("THROW_SOME", expression, evaluation);   \
        }                                                               \
        SCT_INTERNAL_TRACEBACK_RESET                                    \
    } while (0)

#define THROW_SOME_AS(expression, error)                                            \
    do {                                                                            \
        typeof(expression) evaluation = (expression);                               \
        if (evaluation != 0) {                                                      \
            SCT_INTERNAL_THROW_AS("THROW_SOME_AS", expression, evaluation, error);  \
        }                                                                           \
        SCT_INTERNAL_TRACEBACK_RESET                                                \
    } while (0)

#define THROW_CASE(x)                               \
    case (x):                                       \
        SCT_INTERNAL_THROW("THROW_CASE", x, x)

#define THROW_CASE_AS(x, y)                         \
    case (x):                                       \
        SCT_INTERNAL_THROW("THROW_CASE_AS", y, x)

#define SCT_INTERNAL_THROWF(description, error, format, args...)                        \
    do {                                                                                \
        ERRORF(format, ## args);                                                        \
        SCT_INTERNAL_TRACEBACK_PUSHF_WITH_ERROR(description, error, format, ## args)    \
        return error;                                                                   \
    } while (0)

#define THROWF(error, format, args...) SCT_INTERNAL_THROWF("THROWF", format, ## args)

#define THROWF_IF(condition, error, format, args...)                                    \
    do {                                                                                \
        if (condition) {                                                                \
            SCT_INTERNAL_THROWF("THROWF_IF " # condition " ", error, format, ## args);  \
        }                                                                               \
    } while (0)

//
//  CRASH
//

#define SCT_INTERNAL_CRASH(description, expression, evaluation)             \
    do {                                                                    \
        SCT_INTERNAL_TRACEBACK_PUSH(description, expression, evaluation)    \
        SCT_INTERNAL_TRACEBACK_PRINT(description, expression, evaluation)   \
        exit(1);                                                            \
    } while (0)

#define CRASH(expression)                                       \
    do {                                                        \
        typeof(expression) evaluation = (expression);           \
        SCT_INTERNAL_CRASH("CRASH", expression, evaluation);    \
    } while (0)

#define CRASH_IF(condition)                                         \
    do {                                                            \
        typeof(condition) evaluation = (condition);                 \
        if (evaluation) {                                           \
            SCT_INTERNAL_CRASH("CRASH_IF", condition, evaluation);  \
        }                                                           \
        SCT_INTERNAL_TRACEBACK_RESET                                \
    } while (0)

#define CRASH_SOME(expression)                                          \
    do {                                                                \
        typeof(expression) evaluation = (expression);                   \
        if (evaluation != 0) {                                          \
            SCT_INTERNAL_CRASH("CRASH_SOME", expression, evaluation);   \
        }                                                               \
        SCT_INTERNAL_TRACEBACK_RESET                                    \
    } while (0)

#define CRASH_CASE(x)                           \
    case (x):                                   \
        SCT_INTERNAL_CRASH("CRASH_CASE", x, x)

#define SCT_INTERNAL_CRASHF(description, format, args...)           \
    do {                                                            \
        SCT_INTERNAL_TRACEBACK_PUSHF(description, format, ## args)  \
        SCT_INTERNAL_TRACEBACK_PRINTF(description, format, ## args) \
        exit(1);                                                    \
    } while (0)

#define CRASHF(format, args...) SCT_INTERNAL_CRASHF("CRASHF", format, ## args)

#define CRASHF_IF(condition, format, args...)                                       \
    do {                                                                            \
        if (condition) {                                                            \
            SCT_INTERNAL_CRASHF("CRASHF_IF " # condition " ", format, ## args);     \
        }                                                                           \
    } while (0)

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
