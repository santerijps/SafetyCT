#ifndef SAFETYCT_H
#define SAFETYCT_H

#include <stdio.h>
#include <stdlib.h>

#define __concat_name(PREFIX, SUFFIX) PREFIX ## SUFFIX
#define concat_name(PREFIX, SUFFIX) __concat_name(PREFIX, SUFFIX)
#define unique_name(PREFIX) concat_name(PREFIX, __COUNTER__)

#ifdef DEBUG
    #define __traceback_count_max 128
    #define __traceback_length_max 256
    #define __traceback_highlight_color "\e[31m"
    #define __traceback_default_color "\e[0m"
    #define __traceback_format "    File %s, line %d, in function %s\n        %s %s"

    static int __traceback_count = 0;
    static char __traceback[__traceback_count_max][__traceback_length_max];

    #define __traceback_reset __traceback_count = 0;

    #define __traceback_push(FILE, LINE, FUNCTION, CAUSE, ERROR)    \
        if (__traceback_count < __traceback_count_max) {            \
            snprintf(                                               \
                (char*) &__traceback[__traceback_count++],          \
                __traceback_length_max,                             \
                __traceback_format,                                 \
                FILE, LINE, FUNCTION, CAUSE, # ERROR                \
            );                                                      \
        }

    #define __traceback_print                                           \
        do {                                                            \
            fprintf(stderr, "Traceback (most recent call last):\n");    \
            for (int i = 0; i < __traceback_count; i += 1)              \
                fprintf(stderr, "%s\n", __traceback[i]);                \
        } while (0);
#else
    #define __traceback_reset
    #define __traceback_push(FILE, LINE, FUNCTION, CAUSE, ERROR)
    #define __traceback_print
#endif

// Returns the pointer if it's not null, otherwise crashes the program.
#define null_safe(POINTER)                  \
    ({                                      \
        if ((POINTER) == NULL) {            \
            __crash("null_safe", POINTER);  \
        }                                   \
        (POINTER);                          \
    })

#define __throw(CAUSE, ERROR)                                                       \
    do {                                                                            \
        __traceback_push(__FILE__, __LINE__, __PRETTY_FUNCTION__, CAUSE, ERROR)     \
        error = ERROR;                                                              \
        goto exit;                                                                  \
    } while (0)

// Throw an error.
#define throw(ERROR) __throw("throw", ERROR)

#define __crash(CAUSE, ERROR)                                                       \
    do {                                                                            \
        __traceback_push(__FILE__, __LINE__, __PRETTY_FUNCTION__, CAUSE, ERROR)     \
        __traceback_print                                                           \
        exit(EXIT_FAILURE);                                                         \
    } while (0)

// Crash the program with an error.
#define crash(ERROR) __crash("crash", ERROR)

#define __remark(ERROR, VARIABLE_NAME)  \
    do {                                \
        int VARIABLE_NAME = ERROR;      \
        if (VARIABLE_NAME != 0) {       \
            error = VARIABLE_NAME;      \
        }                               \
    } while (0)

// Remark the error but do not throw or crash.
#define remark(ERROR) __remark(ERROR, unique_name(__error))

#define __try(ERROR, VARIABLE_NAME)     \
    do {                                \
        int VARIABLE_NAME = ERROR;      \
        if (VARIABLE_NAME != 0) {       \
            __throw("try", ERROR);      \
        }                               \
    } while (0)

// Try to successfully run a SafetyCT function.
// Throw any error that occurs, continue if there is no error.
// This can be used if it's not important to handle every error individually.
#define try(ERROR) __try(ERROR, unique_name(__error))

#define __expect(ERROR, VARIABLE_NAME)  \
    do {                                \
        int VARIABLE_NAME = ERROR;      \
        if (VARIABLE_NAME != 0) {       \
            __crash("expect", ERROR);   \
        }                               \
        __traceback_reset               \
    } while (0)

// Attempt to successfully run a SafetyCT function.
// Crash if an error occurs, continue if there is no error.
// This can be used like a runtime assert, asserting that the function call must succeed.
#define expect(ERROR) __expect(ERROR, unique_name(error))

// Assume that the expression is truthy, throw if it's not.
// The `error` variable is set to `ERROR` and `goto exit` is performed.
#define assume(EXPRESSION, ERROR)       \
    do {                                \
        if (!(EXPRESSION)) {            \
            __throw("assume", ERROR);   \
        }                               \
    } while (0)

// Presume that the expression is truthy, crash if it's not.
// This can be used like a runtime assert, asserting that the expression is truthy.
#define presume(EXPRESSION, ERROR)      \
    do {                                \
        if (!(EXPRESSION)) {            \
            __crash("presume", ERROR);  \
        }                               \
        __traceback_reset               \
    } while (0)

// Ignore the error and resume with the rest of the function.
// The `error` variable will not be updated.
#define ignore(ERROR)   \
    case ERROR:         \
        break

// Update the `error` variable and resume with the rest of the function.
// Use cases: when there is no error or the error is not fatal.
#define resume(ERROR)   \
    case ERROR:         \
        error = ERROR;  \
        break

// Propagate an error to the calling function instead of handling it yourself.
#define propagate(ERROR)            \
    case ERROR:                     \
        __throw("propagate", ERROR)

// Propagate an error as a different error to the calling function instead of handling it yourself.
#define propagate_as(ERROR_UPSTREAM, ERROR_DOWNSTREAM)  \
    case ERROR_UPSTREAM:                                \
        __throw("propagate", ERROR_DOWNSTREAM)

// Refuse to deal with the error and crash the program.
// Call this in an error-handler `switch` in situations where the error should never occur.
// Only use this if you know that the error should never be returned!
#define refuse(ERROR)               \
    case ERROR:                     \
        __crash("refuse", ERROR)

#define __defer_if(CONDITION, STATEMENT, CLEANUP_VARIABLE_NAME, CLEANUP_FUNCTION_NAME)  \
    void CLEANUP_FUNCTION_NAME(void *arg) {                                             \
        (void) arg;                                                                     \
        if (CONDITION) {                                                                \
            STATEMENT;                                                                  \
        }                                                                               \
    }                                                                                   \
    int CLEANUP_VARIABLE_NAME __attribute__ ((__cleanup__(CLEANUP_FUNCTION_NAME))) = 0

// Defer running statements until the end of the current scope.
// If there are multiple defers in the same scope, they will be called in reverse order.
#define defer(STATEMENT) __defer_if(1, STATEMENT, unique_name(__cleanup_var),  unique_name(__cleanup_func))

// Defer running statements until the end of the current scope if the condition is truthy.
// If there are multiple defers in the same scope, they will be called in reverse order.
#define defer_if(CONDITION, STATEMENT) __defer_if(CONDITION, STATEMENT, unique_name(__cleanup_var),  unique_name(__cleanup_func))

#endif
