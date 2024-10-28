#include "../../safetyct.h"

enum ReadFileError {
    READ_FILE_ERROR_NONE,
    READ_FILE_ERROR_NULL_PATH,
    READ_FILE_ERROR_NULL_BUFFER,
    READ_FILE_ERROR_ZERO_SIZE,
    READ_FILE_ERROR_OPEN_FAILED,
    READ_FILE_ERROR_READ_FAILED,
    READ_FILE_ERROR_BUFFER_TOO_SMALL,
};

enum ReadFileError read_file(
    const char *file_path,
    char *buffer,
    size_t size,
    size_t *bytes_read
) {
    enum ReadFileError error = READ_FILE_ERROR_NONE;
    assume(file_path != NULL, READ_FILE_ERROR_NULL_PATH);
    assume(buffer != NULL, READ_FILE_ERROR_NULL_BUFFER);
    assume(size > 0, READ_FILE_ERROR_ZERO_SIZE);

    FILE *file = fopen(file_path, "r");
    assume(file != NULL, READ_FILE_ERROR_OPEN_FAILED);
    defer(fclose(file));

    size_t n = fread_s(buffer, size, 1, size - 1, file);
    assume(!ferror(file), READ_FILE_ERROR_READ_FAILED);
    assume(feof(file), READ_FILE_ERROR_BUFFER_TOO_SMALL);

    if (bytes_read != NULL) {
        *bytes_read = n;
    }

    return error;
}

test("read_file fails when file_path is null", {
    const char *file_path = NULL;
    char buffer[16] = {0};
    size_t bytes_read = 0;
    enum ReadFileError error = read_file(file_path, buffer, len(buffer), &bytes_read);
    assert_equal(error, READ_FILE_ERROR_NULL_PATH);
});

test("read_file fails when buffer is null", {
    const char *file_path = "input.txt";
    size_t bytes_read = 0;
    enum ReadFileError error = read_file(file_path, NULL, 0, &bytes_read);
    assert_equal(error, READ_FILE_ERROR_NULL_BUFFER);
});

test("read_file fails when size is 0", {
    const char *file_path = "input.txt";
    char buffer[16] = {0};
    size_t bytes_read = 0;
    enum ReadFileError error = read_file(file_path, buffer, 0, &bytes_read);
    assert_equal(error, READ_FILE_ERROR_ZERO_SIZE);
});

test("read_file fails when file doesn't exist", {
    const char *file_path = "not_found.txt";
    char buffer[16] = {0};
    size_t bytes_read = 0;
    enum ReadFileError error = read_file(file_path, buffer, len(buffer), &bytes_read);
    assert_equal(error, READ_FILE_ERROR_OPEN_FAILED);
});

test("read_file happy case", {
    const char *file_path = "input.txt";
    char buffer[16] = {0};
    size_t bytes_read = 0;
    enum ReadFileError error = read_file(file_path, buffer, len(buffer), &bytes_read);
    assert_equal(error, READ_FILE_ERROR_NONE);
});

test("random", {
    assert_equal(1 + 1, 3);
})

int main(void) {
    puts("This example demonstrates testing your code. To run the tests, compile the program with the -DTEST flag.");
    return 0;
}
