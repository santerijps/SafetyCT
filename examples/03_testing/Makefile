ifeq ($(OS),Windows_NT)
    OUT := out.exe
else
    OUT := out
endif

build:
	gcc main.c -o $(OUT) -Wall -Wextra -Werror -DDEBUG -DTEST
