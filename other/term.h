/*
    References:
        - https://en.wikipedia.org/wiki/ANSI_escape_code
        - https://gist.github.com/ConnerWill/d4b6c776b509add763e17f9f113fd25b
        - https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences
        - https://theasciicode.com.ar/

    Macros starting with TERM_ are statements that call a function.
    Macros starting with TERMS_ always evaluate to a string literal.

    Regarding graphics:
        When changing terminal graphics with printf, put the graphics macros
        between the TERMS_GRAPHICS_BEGIN and TERMS_GRAPHICS_BEGIN macros.

        For example:
            printf(
                TERMS_GRAPHICS_BEGIN
                    TERMS_BOLD
                    TERMS_FG_RED
                TERMS_GRAPHICS_END
                "Hello, world!\n"
                TERMS_GRAPHICS_BEGIN
                    TERMS_NO_BOLD
                    TERMS_ITALIC
                TERMS_GRAPHICS_END
                "What's up?\n"
            );

        This is not required if you use the printing macros provided in this file.

    NOTE: You cannot move the cursor and set graphics at the same time!
    You have to do them separately, i.e. you cannot write graphics and change the cursor position in the same write call.
*/

#ifndef SAFETYCT_TERM_H
#define SAFETYCT_TERM_H

#include <stdio.h>

//
//  PRINTING
//

#define TERM_FPRINTF(graphics_literal, file, format, args...)\
    fprintf(file, TERMS_GRAPHICS_BEGIN graphics_literal TERMS_GRAPHICS_END format "\e[0m", ## args)
#define TERM_PRINTF(graphics_literal, format, args...) TERM_FPRINTF(graphics_literal, stdout, format, ## args)

#define TERM_FWRITE(file, string, size) fwrite(string, 1, size, file)
#define TERM_WRITE(string, size) TERM_FWRITE(stdout, string, size)

#define TERM_FWRITE_LITERAL(file, string) TERM_FWRITE(file, string, sizeof(string))
#define TERM_WRITE_LITERAL(string) TERM_FWRITE_LITERAL(stdout, string)

//
//  GRAPHICS
//

#define TERMS_GRAPHICS_BEGIN "\e[255"
#define TERMS_GRAPHICS_END "m"
#define TERMS_GRAPHICS_RESET "\e[0m"

#define TERMS_DEFAULT ";0"
#define TERMS_BOLD ";1"
#define TERMS_DIM ";2"
#define TERMS_ITALIC ";3"
#define TERMS_UNDERLINE ";4"
#define TERMS_BLINKING ";5"
#define TERMS_INVERSE ";7"
#define TERMS_HIDDEN ";8"
#define TERMS_STRIKETHROUGH ";9"

#define TERMS_NO_BOLD ";21"
#define TERMS_NO_DIM ";22"
#define TERMS_NO_ITALIC ";23"
#define TERMS_NO_UNDERLINE ";24"
#define TERMS_NO_BLINKING ";25"
#define TERMS_NO_INVERSE ";27"
#define TERMS_NO_HIDDEN ";28"
#define TERMS_NO_STRIKETHROUGH ";29"

#define TERMS_FG_BLACK ";30"
#define TERMS_FG_RED ";31"
#define TERMS_FG_GREEN ";32"
#define TERMS_FG_YELLOW ";33"
#define TERMS_FG_BLUE ";34"
#define TERMS_FG_MAGENTA ";35"
#define TERMS_FG_CYAN ";36"
#define TERMS_FG_WHITE ";37"
#define TERMS_FG_DEFAULT ";39"

#define TERMS_BG_BLACK ";40"
#define TERMS_BG_RED ";41"
#define TERMS_BG_GREEN ";42"
#define TERMS_BG_YELLOW ";43"
#define TERMS_BG_BLUE ";44"
#define TERMS_BG_MAGENTA ";45"
#define TERMS_BG_CYAN ";46"
#define TERMS_BG_WHITE ";47"
#define TERMS_BG_DEFAULT ";49"

#define TERMS_FG_RGB(red, green, blue) ";38;2;" # red ";" # green ";" # blue
#define TERMS_BG_RGB(red, green, blue) ";48;2;" # red ";" # green ";" # blue

#define TERMS_FG_256(code) ";38;5;" code
#define TERMS_BG_256(code) ";48;5;" code

#define TERM_GRAPHICS_SET(string_literal) TERM_WRITE_LITERAL(TERMS_GRAPHICS_BEGIN string_literal TERMS_GRAPHICS_END)
#define TERM_GRAPHICS_RESET() TERM_WRITE_LITERAL(TERMS_GRAPHICS_RESET)

//
//  OTHER
//

#define TERMS_WINDOW_TITLE_SET(title) "\e]0;" title "\x07"

#define TERMS_ALT_BUFFER_ENABLE "\e[?1049h"
#define TERMS_ALT_BUFFER_DISABLE "\e[?1049l"

#define TERMS_CLEAR_AFTER_CURSOR "\e[0J"
#define TERMS_CLEAR_BEFORE_CURSOR "\e[1J"
#define TERMS_CLEAR "\e[2J"

#define TERMS_CLEAR_LINE_AFTER_CURSOR "\e[0K"
#define TERMS_CLEAR_LINE_BEFORE_CURSOR "\e[1K"
#define TERMS_CLEAR_LINE "\e[2K"

#define TERM_CLEAR_AFTER_CURSOR() TERM_WRITE_LITERAL(TERMS_CLEAR_AFTER_CURSOR)
#define TERM_CLEAR_BEFORE_CURSOR() TERM_WRITE_LITERAL(TERMS_CLEAR_BEFORE_CURSOR)
#define TERM_CLEAR() TERM_WRITE_LITERAL(TERMS_CLEAR)

#define TERM_CLEAR_LINE_AFTER_CURSOR() TERM_WRITE_LITERAL(TERMS_CLEAR_LINE_AFTER_CURSOR);
#define TERM_CLEAR_LINE_BEFORE_CURSOR() TERM_WRITE_LITERAL(TERMS_CLEAR_LINE_BEFORE_CURSOR);
#define TERM_CLEAR_LINE() TERM_WRITE_LITERAL(TERMS_CLEAR_LINE);

//
//  CURSOR
//

#define TERMS_CURSOR_POS_HOME "\e[H"
#define TERMS_CURSOR_POS_SET(line, column) "\e[" # line ";" # column "H"

#define TERMS_CURSOR_MOVE_UP(n) "\e[" # n "A"
#define TERMS_CURSOR_MOVE_DOWN(n) "\e[" # n "B"
#define TERMS_CURSOR_MOVE_RIGHT(n) "\e[" # n "C"
#define TERMS_CURSOR_MOVE_LEFT(n) "\e[" # n "D"

#define TERMS_CURSOR_MOVE_DOWN_HOME(n) "\e[" # n "E"
#define TERMS_CURSOR_MOVE_UP_HOME(n) "\e[" # n "F"

#define TERMS_CURSOR_COLUMN_SET(n) "\e[" # n "G"

#define TERMS_CURSOR_MOVE_UP_SCROLL "\eM"

#define TERMS_CURSOR_SAVE "\e7" // or s
#define TERMS_CURSOR_RESTORE "\e8" // or u

__attribute__((destructor))
static void scti_destructor_reset_terminal_graphics(void) {
    TERM_GRAPHICS_RESET();
}

#endif
