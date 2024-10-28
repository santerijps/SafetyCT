# SafetyCT

SafetyCT ("safety seat") is a collection of macros in a [single header file](safetyct.h) to help with common error handling tasks and safety concerns when programming in the C programming language. You will also find a set of recommended compiler options to use in your builds.

For code examples, see [the examples directory](./examples).

## Features

The single header file includes:

- Macros to elegantly handle errors instead of typing out many `if` -statements and long `switch` -statements
- Alternatives for runtime asserts with more detailed error messages
- The `defer` and `defer_if` macros to defer running code until the end of the current scope
- Python -like traceback messages to simplify the debugging process (behind the `-DDEBUG` compiler option)
- Macros that allow writing tests in the source code (the tests can be run with the `-DTEST` compiler option)

## Motivation

Writing functions that can fail in many different ways poses a challenge in ensuring that all the possible corner cases are handled appropriately.
Personally, I began to appreciate the "error code as a return value" approach, as I could then easily check in an if-statement if the function call was successful,
or alternatively, handle each specific error code in a switch-case-statement. There are many different ways to handle errors in C, you should definitiely read [Ethan McCue's article][3] to get an idea of what we're dealing with.

The "error code as a return value" approach, however, is very verbose. Although I enjoy writing out transparent and obvious C code, I thought that it would not be out of place to implement some macros to help out with the repetetive tasks. I have a love-hate relationship with macros, as I can appreciate the simple ones that only do one or two things, but the more complex the macro is, the more I begin to despise it. Hence all the macros in SafetyCT should be small, and only do at most three things.

### Modern languages as an inspiration

Error handling is a very important topic in the C landscape, as C barely provides any language features to help us in this matter. Many modern languages, on the other hand, offer great features to do exactly what C does not. Zig, for example, has error types and the `try` and `catch` keywords to natively get the job done. In Zig, if you call a function that might return an error, the compiler forces you to handle the error in some way, such as using the `try` keyword to propagate the error, or `catch` to deal with the error like you would in a switch-case. The safety features found in Zig are a big inspiration for this project.

## Basic concepts

Generally, you want to perform checks on parameters and return values in either `if` or `switch` -statements. Depending on the significance of the check you will either want to perform an early return or exit the program. In the context of SafetyCT, "throwing" refers to early returning and "crashing" refers to exiting the program.

It's also possible to simply update the error variable and continue without throwing or crashing. This is useful when the error is not significant enough to cause an early return nor a program exit, or the error is a "happy" error that communicates some extra info.

To help with these, refer to the following table:

|Statment/operation|if (condition)|if (error)|case|
|-|-|-|-|
|**update error**|-|`remark`|`resume`|
|**throw**|`assume`|`try`|`propagate`|
|**crash**|`presume`|`expect`|`refuse`|

*Example 1: You want to throw an error if one of the function parameters is NULL. A simple if-check is enough, and you don't want to crash the program. The macro to go with is `assume`.*

*Example 2: You want to crash your program if the return value of a function you called signifies a fatal error. The function returns many different error codes, thus it makes sense to use a switch. The macro to go with is `refuse`.*

*Example 3: You are calling a function that returns an error code (0 means no error) but you know that the function call should never fail. You just want to assume that it runs correctly and carry on. The macro to go with is `expect`.*

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
