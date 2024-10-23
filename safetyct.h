#ifndef SAFETYCT_H
#define SAFETYCT_H

#include <stdio.h>
#include <stdlib.h>

// Returns the pointer if it's not null, otherwise crashes the program.
#define null_safe(POINTER)                      \
    ({                                          \
        if ((POINTER) == NULL) {                \
            fprintf(                            \
                stderr,                         \
                "Unexpected null pointer!\n"    \
                "File:       %s:%d\n"           \
                "Function:   %s\n"              \
                "Pointer:    %s\n",             \
                __FILE__, __LINE__,             \
                __PRETTY_FUNCTION__,            \
                # POINTER                       \
            );                                  \
            exit(EXIT_FAILURE);                 \
        }                                       \
        (POINTER);                              \
    })

// Throw an error by setting the `error` variable to `ERROR`,
// followed by `goto exit`.
//
// Requirements:
//  - `error` variable is declared
//  - `exit` label is declared
#define throw(ERROR)        \
    do {                    \
        error = ERROR;      \
        goto exit;          \
    } while (0)

// Refuse to deal with the error and crash the program.
// Call this in an error-handler `switch` in situations where the error should never occur.
// Only use this if you know that the error should never be returned!
// NOTE: CAUSES THE PROGRAM TO EXIT!
//
// Requirements:
//  - called inside a `switch` -statement
#define refuse(ERROR)                       \
    case ERROR:                             \
        fprintf(                            \
            stderr,                         \
            "Forbidden error occurred!\n"   \
            "File:       %s:%d\n"           \
            "Function:   %s\n"              \
            "Error:      %s(%d)\n",         \
            __FILE__, __LINE__,             \
            __PRETTY_FUNCTION__,            \
            # ERROR, ERROR                  \
        );                                  \
        exit(EXIT_FAILURE)

// Propagate an error to the calling function instead of handling it yourself.
//
// Requirements:
//  - called inside a `switch` -statement
//  - `error` variable is declared
//  - `exit` label is declared
#define propagate(ERROR)    \
    case ERROR:             \
        throw(ERROR)

// Propagate an error as a different error to the calling function instead of handling it yourself.
//
// Requirements:
//  - called inside a `switch` -statement
//  - `error` variable is declared
//  - `exit` label is declared
#define propagate_as(ERROR_UPSTREAM, ERROR_DOWNSTREAM)  \
    case ERROR_UPSTREAM:                                \
        throw(ERROR_DOWNSTREAM)

// Ignore the error and resume with the rest of the function.
// The `error` variable will not be updated.
// Requirements:
//  - called inside a `switch` -statement
#define ignore(ERROR)   \
    case ERROR:         \
        break

// Update the `error` variable and resume with the rest of the function.
// Use cases: when there is no error or the error is not fatal.
// Requirements:
//  - called inside a `switch` -statement
//  - `error` variable is declared
#define resume(ERROR)   \
    case ERROR:         \
        error = ERROR;  \
        break

#define endure resume
#define perish refuse
#define escape propagate
#define escape_as propagate_as

#define __catch(ERROR, VARIABLE_NAME)   \
    do {                                \
        int VARIABLE_NAME = ERROR;      \
        if (VARIABLE_NAME != 0) {       \
            throw(VARIABLE_NAME);       \
        }                               \
    } while (0)

// Try to successfully run a SafetyCT function.
// Throw any error that occurs, continue if there is no error.
// This can be used if it's not important to handle every error individually.
#define catch(ERROR) __catch(ERROR, unique_name(__error))

// Assume that the expression is truthy, throw if it's not.
// The `error` variable is set to `ERROR` and `goto exit` is performed.
//
// Requirements:
//  - `error` variable is declared
//  - `exit` label is declared
#define assume(EXPRESSION, ERROR)   \
    do {                            \
        if (!(EXPRESSION)) {        \
            throw(ERROR);           \
        }                           \
    } while (0)

#define __raise(ERROR, VARIABLE_NAME)   \
    do {                                \
        int VARIABLE_NAME = ERROR;      \
        if (VARIABLE_NAME != 0) {       \
            crash(VARIABLE_NAME);       \
        }                               \
    } while (0)

// Attempt to successfully run a SafetyCT function.
// Crash if an error occurs, continue if there is no error.
// This can be used like a runtime assert, asserting that the function call must succeed.
#define raise(ERROR) __raise(ERROR, unique_name(error))

// Presume that the expression is truthy, crash if it's not.
// This can be used like a runtime assert, asserting that the expression is truthy.
#define presume(EXPRESSION, ERROR)  \
    do {                            \
        if (!(EXPRESSION)) {        \
            crash(ERROR);           \
        }                           \
    } while (0)

// Declare a part of your source code unreachable.
// In case it's reached, it should be considered a serious bug!
// NOTE: CAUSES THE PROGRAM TO EXIT!
#define unreachable                         \
    do {                                    \
        fprintf(                            \
            stderr,                         \
            "Reached unreachable code!\n"   \
            "File:       %s:%d\n"           \
            "Function:   %s\n",             \
            __FILE__, __LINE__,             \
            __PRETTY_FUNCTION__             \
        );                                  \
        exit(EXIT_FAILURE);                 \
    } while (0)

// Crash the program with an error.
#define crash(ERROR)                        \
    do {                                    \
        fprintf(                            \
            stderr,                         \
            "Crash occurred!\n"             \
            "File:       %s:%d\n"           \
            "Function:   %s\n"              \
            "Error:      %d\n",             \
            __FILE__, __LINE__,             \
            __PRETTY_FUNCTION__,            \
            ERROR                           \
        );                                  \
        exit(EXIT_FAILURE);                 \
    } while (0)

#define __concat_name(PREFIX, SUFFIX) PREFIX ## SUFFIX
#define concat_name(PREFIX, SUFFIX) __concat_name(PREFIX, SUFFIX)
#define unique_name(PREFIX) concat_name(PREFIX, __COUNTER__)

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
#define defer(STATEMENT)\
    __defer_if(1, STATEMENT, unique_name(__cleanup_var),  unique_name(__cleanup_func))

// Defer running statements until the end of the current scope if the condition is truthy.
// If there are multiple defers in the same scope, they will be called in reverse order.
#define defer_if(CONDITION, STATEMENT)\
    __defer_if(CONDITION, STATEMENT, unique_name(__cleanup_var),  unique_name(__cleanup_func))

#endif
