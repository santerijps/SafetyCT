// https://en.wikipedia.org/wiki/ANSI_escape_code
// https://gist.github.com/ConnerWill/d4b6c776b509add763e17f9f113fd25b
// https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences
// https://theasciicode.com.ar/

// NOTE: You cannot move the cursor and set graphics at the same time! You have to do them separately.

#ifndef SAFETYCT_TERM_H
#define SAFETYCT_TERM_H

#include <stdio.h>

//
//  GRAPHICS
//

#define TERM_STRING_DEFAULT ";0"
#define TERM_STRING_BOLD ";1"
#define TERM_STRING_DIM ";2"
#define TERM_STRING_ITALIC ";3"
#define TERM_STRING_UNDERLINE ";4"
#define TERM_STRING_BLINKING ";5"
#define TERM_STRING_INVERSE ";7"
#define TERM_STRING_HIDDEN ";8"
#define TERM_STRING_STRIKETHROUGH ";9"

#define TERM_STRING_NO_BOLD ";21"
#define TERM_STRING_NO_DIM ";22"
#define TERM_STRING_NO_ITALIC ";23"
#define TERM_STRING_NO_UNDERLINE ";24"
#define TERM_STRING_NO_BLINKING ";25"
#define TERM_STRING_NO_INVERSE ";27"
#define TERM_STRING_NO_HIDDEN ";28"
#define TERM_STRING_NO_STRIKETHROUGH ";29"

#define TERM_STRING_FG_BLACK ";30"
#define TERM_STRING_FG_RED ";31"
#define TERM_STRING_FG_GREEN ";32"
#define TERM_STRING_FG_YELLOW ";33"
#define TERM_STRING_FG_BLUE ";34"
#define TERM_STRING_FG_MAGENTA ";35"
#define TERM_STRING_FG_CYAN ";36"
#define TERM_STRING_FG_WHITE ";37"
#define TERM_STRING_FG_DEFAULT ";39"

#define TERM_STRING_BG_BLACK ";40"
#define TERM_STRING_BG_RED ";41"
#define TERM_STRING_BG_GREEN ";42"
#define TERM_STRING_BG_YELLOW ";43"
#define TERM_STRING_BG_BLUE ";44"
#define TERM_STRING_BG_MAGENTA ";45"
#define TERM_STRING_BG_CYAN ";46"
#define TERM_STRING_BG_WHITE ";47"
#define TERM_STRING_BG_DEFAULT ";49"

#define TERM_STRING_FG_RGB(red, green, blue) ";38;2;" # red ";" # green ";" # blue
#define TERM_STRING_BG_RGB(red, green, blue) ";48;2;" # red ";" # green ";" # blue

#define TERM_STRING_FG_256(code) ";38;5;" code
#define TERM_STRING_BG_256(code) ";48;5;" code

#define TERM_STRING_GRAPHICS_BEGIN "\e[255"
#define TERM_STRING_GRAPHICS_END "m"
#define TERM_STRING_GRAPHICS_RESET "\e[0m"

#define TERM_FPRINTF(file, format, args...) fprintf(file, format "\e[0m", ## args)
#define TERM_PRINTF(format, args...) TERM_FPRINTF(stdout, format, ## args)

#define TERM_FWRITE(file, string, size) fwrite(string, 1, size, file)
#define TERM_WRITE(string, size) TERM_FWRITE(stdout, string, size)

#define TERM_FWRITE_LITERAL(file, string) TERM_FWRITE(file, string, sizeof(string))
#define TERM_WRITE_LITERAL(string) TERM_FWRITE_LITERAL(stdout, string)

#define TERM_GRAPHICS_SET(string_codes) TERM_WRITE(TERM_STRING_GRAPHICS_BEGIN string_codes TERM_STRING_GRAPHICS_END)
#define TERM_GRAPHICS_RESET() TERM_WRITE(TERM_STRING_GRAPHICS_RESET, sizeof(TERM_STRING_GRAPHICS_RESET))

//
//  OTHER
//

#define TERM_STRING_WINDOW_TITLE(title) "\e]0;" title "\x07"

#define TERM_STRING_ALT_BUFFER_ENABLE "\e[?1049h"
#define TERM_STRING_ALT_BUFFER_DISABLE "\e[?1049l"

#define TERM_STRING_CLEAR_AFTER_CURSOR "\e[0J"
#define TERM_STRING_CLEAR_BEFORE_CURSOR "\e[1J"
#define TERM_STRING_CLEAR "\e[2J"

#define TERM_STRING_CLEAR_LINE_AFTER_CURSOR "\e[0K"
#define TERM_STRING_CLEAR_LINE_BEFORE_CURSOR "\e[1K"
#define TERM_STRING_CLEAR_LINE "\e[2K"

//
//  CURSOR
//

#define TERM_STRING_CURSOR_POS_HOME "\e[H"
#define TERM_STRING_CURSOR_POS_SET(line, column) "\e[" # line ";" # column "H"

#define TERM_STRING_CURSOR_MOVE_UP(n) "\e[" # n "A"
#define TERM_STRING_CURSOR_MOVE_DOWN(n) "\e[" # n "B"
#define TERM_STRING_CURSOR_MOVE_RIGHT(n) "\e[" # n "C"
#define TERM_STRING_CURSOR_MOVE_LEFT(n) "\e[" # n "D"

#define TERM_STRING_CURSOR_MOVE_DOWN_HOME(n) "\e[" # n "E"
#define TERM_STRING_CURSOR_MOVE_UP_HOME(n) "\e[" # n "F"

#define TERM_STRING_CURSOR_COLUMN_SET(n) "\e[" # n "G"

#define TERM_STRING_CURSOR_MOVE_UP_SCROLL "\eM"

#define TERM_STRING_CURSOR_SAVE "\e7" // or s
#define TERM_STRING_CURSOR_RESTORE "\e8" // or u

__attribute__((destructor))
static void sct_internal_destructor_reset_terminal_graphics(void) {
    TERM_GRAPHICS_RESET();
}

#endif
