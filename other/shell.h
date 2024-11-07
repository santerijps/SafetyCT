#ifndef SAFETYCT_SHELL_H
#define SAFETYCT_SHELL_H

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @name SHELL
 * @brief Run a shell command.
 * @param command String literal.
 * @return Return value of the `system` function.
 */
#define SHELL(command) system("sh -c \"" command "\"")

/**
 * @name SHELL_BUFFER_SIZE
 * @brief The maximum buffer size for the `shell` function.
 */
#define SHELL_BUFFER_SIZE 1024

/**
 * @name shell
 * @brief Run a shell command.
 * @param command String literal or a pointer.
 * @return Return value of the `system` function.
 */
static inline int shell(const char* const command) {
    char const buffer[SHELL_BUFFER_SIZE] = {0};
    sprintf_s((char*)buffer, SHELL_BUFFER_SIZE, "sh -c \"%s\"", command);
    return system(buffer);
}

/**
 * @name CONFIRM_FORMAT
 * @brief The format to use in the confirm function.
 */
#define CONFIRM_FORMAT "%s (y/n/q) "

/**
 * @name confirm
 * @brief Asks for the user's confirmation on the command line.
 * @param message A message to display to the user.
 * @return True or false depending on whether the user confirmed or not.
 */
static inline int confirm(const char * const message) {
    printf(CONFIRM_FORMAT, message);
    for (;;) {
        int byte = getch();
        switch (byte) {
            case 'y':
            case 'Y':
                printf("%c\n", byte);
                return 1;
            case 'n':
            case 'N':
                printf("%c\n", byte);
                return 0;
            case 'q':
            case 'Q':
            case 3: // ctrl+c
                exit(1);
            default:
                continue;
        }
    }
    return 0;
}

/**
 * @name prompt
 * @brief Ask the user for input on the command line.
 * @param message A message to display to the user.
 * @param buffer The buffer to write the user's input to.
 * @param buffer_size The size of the buffer.
 * @return True if all user input was read.
 */
static inline int prompt(
    const char* const message,
    char* const buffer,
    size_t const buffer_size
) {
    printf("%s", message);
    fgets(buffer, buffer_size - 1, stdin);
    for (long long i = buffer_size - 1; i >= 0; i -= 1) {
        if (buffer[i] != '\0') {
            if (buffer[i] == '\n') {
                buffer[i] = '\0';
            }
            break;
        }
    }
    return feof(stdin);
}

#endif
