# SafetyCT

SafetyCT ("safety seat") is a collection of macros and functions in a [single header file](safetyct.h) to help with common error handling tasks and safety concerns when programming in the C programming language. You will also find a set of recommended compiler options to use in your builds.

## Motivation

Writing functions that can fail in many different ways poses a challenge in ensuring that all the possible corner cases are handled appropriately.
Personally, I began to appreciate the "error code as a return value" approach, as I could then easily check in an if-statement if the function call was successful,
or alternatively, handle each specific error code in a switch-case-statement. There are many different ways to handle errors in C, you should definitiely read [Ethan McCue's article][3] to get an idea of what we're dealing with.

The "error code as a return value" approach, however, is very verbose. Although I enjoy writing out transparent and obvious C code, I thought that it would not be out of place to implement some macros to help out with the repetetive tasks. I have a love-hate relationship with macros, as I can appreciate the simple ones that only do one or two things, but the more complex the macro is, the more I begin to despise it. Hence all the macros in SafetyCT should be small, and only do at most three things.

### Modern languages as an inspiration

Error handling is a very important topic in the C landscape, as C barely provides any language features to help us in this matter. Many modern languages, on the other hand, offer great features to do exactly what C does not. Zig, for example, has error types and the `try` and `catch` keywords to natively get the job done. In Zig, if you call a function that might return an error, the compiler forces you to handle the error in some way, such as using the `try` keyword to propagate the error, or `catch` to deal with the error like you would in a switch-case. The safety features found in Zig are a big inspiration for this project.

## SafetyCT compliant function structure

The pattern to implement a SafetyCT complian function is the following:

1. Declare a variable `error` at the top and initialize it to 0. The data type should be `enum`.
2. Declare a label `exit` at the bottom of the function. This label will be `goto`'d in case of an early return.
3. Return the `error` variable declared earlier as the last statement in the function. This should be the only return-statement in the function.
4. The actual function logic happens between the `error` variable and the `exit` label.
5. If this function calls another SafetyCT compliant function, it should handle the returned error in a switch.
    - The errors should only be handled in the switch! No other function logic should be used in the case-statements.
    - This aligns with [Railway Oriented Programming][4].

And that's it! Check out the example below:

```c
enum ReadNumberError {
    READ_NUMBER_ERROR_NONE,
    READ_NUMBER_ERROR_NULL_PATH,
    READ_NUMBER_ERROR_NULL_BUFFER,
    READ_NUMBER_ERROR_FOPEN_FAILED,
    READ_NUMBER_ERROR_BUFFER_EXCEEDED,
    READ_NUMBER_ERROR_INVALID_BYTE,
};


// This function opens up a text file for reading and reads the contents into a buffer.
// The function will only accept the file content to contain digits, or it will return an error.
// There are many other errors that may occur regarding file i/o.
enum ReadNumberError read_number(
    const char *file_path,
    char *buffer,
    size_t buffer_size,
    size_t *invalid_index
) {
    enum ReadNumberError error = READ_NUMBER_ERROR_NONE;
    FILE *file = NULL;
    size_t bytes_read = 0;

    expect(file_path != NULL, READ_NUMBER_ERROR_NULL_PATH);
    expect(buffer != NULL, READ_NUMBER_ERROR_NULL_BUFFER);

    file = fopen(file_path, "r");
    expect(file != NULL, READ_NUMBER_ERROR_FOPEN_FAILED);

    bytes_read = fread(buffer, 1, buffer_size - 1, file);
    expect(feof(file), READ_NUMBER_ERROR_BUFFER_EXCEEDED);

    for (size_t i = 0; i < bytes_read; i += 1) {
        char byte = buffer[i];

        if (byte < '0' || byte > '9') {
            if (invalid_index != NULL) {
                *invalid_index = i;
            }
            throw(READ_NUMBER_ERROR_INVALID_BYTE);
        }
    }

exit:
    fclose(file);
    return error;
}

// Just to test the read_number function and demonstrate some of the macros.
enum ReadNumberError test_read_number(void) {
    enum ReadNumberError error = READ_NUMBER_ERROR_NONE;
    char buffer[256] = {0};
    size_t invalid_index;

    switch (read_number("digits.txt", buffer, len(buffer), &invalid_index)) {
        ignore(READ_NUMBER_ERROR_NONE);
        forbid(READ_NUMBER_ERROR_NULL_PATH);
        forbid(READ_NUMBER_ERROR_NULL_BUFFER);
        propagate(READ_NUMBER_ERROR_FOPEN_FAILED);
        propagate(READ_NUMBER_ERROR_BUFFER_EXCEEDED);

        case READ_NUMBER_ERROR_INVALID_BYTE:
            printf("Error: invalid byte '%c' at index %llu!\n", buffer[invalid_index], invalid_index);
            throw(READ_NUMBER_ERROR_INVALID_BYTE);
    }

    printf("Ok: %s\n", buffer);

exit:
    return error;
}
```

## Where to use what

The macros are named in a very human readable way to help keep the code clear and obvious in what it's trying to achieve.
The SafetyCT idiom does require that it is followed correctly, or else it won't work.
Below are listed all the macros, their descriptions and where you're "allowed" to use them.

### Use in a SafetyCT function

These macros depend on the `exit` label.

- `throw(ERROR)` - Update the error variable and goto exit. "Throwing" in the SafetyCT context refers to this macro.
- `expect(EXPRESSION, ERROR)` - Expect an expression to be true, else throw.
- `catch(EXPRESSION, ERROR)` - Catch a bad expression and throw, else continue with the function.

- The following macros produce a `case`-statement, and thus must only be used inside a `switch`:

    - `ignore(ERROR)` - Break from the `switch` and continue the function.
    - `permit(ERROR)` - Update the error variable, break from the `switch` and continue the function.
    - `propagate(ERROR)` - Update the error variable and goto exit.
    - `propagate_as(ERROR_UPSTREAM, ERROR_DOWNSTREAM)` - Update the error variable and goto exit.
    - `forbid(ERROR)` - Crash the program with a specific error.

### Use anywhere

- `crash(ERROR)` - Crash the program with a specific error.
- `unreachable` - Crash the program without an error.
- `null_safe(POINTER)` - If the pointer is `null`, crash the program. Otherwise return the pointer.

## Recommended compiler options

There are too many options to cover them all here, but check out the following links for more information:

- [GCC warning options][1] (Official GCC documentation)
- [Compiler Options Hardening Guide for C and C++][2] (OpenSSF, 2024-10-17)

To get started with Safety CT, we're mostly concerned with the warnings that enforce safer switches. You can use the following compiler options:

```txt
-Wall -Wextra -Werror -Wswitch-enum -Wimplicit-fallthrough -Wnull-dereference -Wshadow
```

[1]: https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
[2]: https://best.openssf.org/Compiler-Hardening-Guides/Compiler-Options-Hardening-Guide-for-C-and-C++.html
[3]: https://mccue.dev/pages/7-27-22-c-errors
[4]: https://fsharpforfunandprofit.com/rop/
