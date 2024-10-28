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

enum ValidateFileError {
    VALIDATE_FILE_ERROR_NONE,
    VALIDATE_FILE_ERROR_OPEN_FAILED,
    VALIDATE_FILE_ERROR_READ_FAILED,
    VALIDATE_FILE_ERROR_INVALID,
};

enum ValidateFileError validate_file(const char *file_path, size_t *invalid_index) {
    enum ValidateFileError error = VALIDATE_FILE_ERROR_NONE;
    char buffer[2048] = {0};
    size_t bytes_read = 0;

    presume(file_path != NULL);
    presume(invalid_index != NULL);

    switch (read_file(file_path, buffer, len(buffer), &bytes_read)) {
        ignore(READ_FILE_ERROR_NONE);
        propagate_as(READ_FILE_ERROR_OPEN_FAILED, VALIDATE_FILE_ERROR_OPEN_FAILED);
        propagate_as(READ_FILE_ERROR_READ_FAILED, VALIDATE_FILE_ERROR_READ_FAILED);
        refuse(READ_FILE_ERROR_NULL_PATH);
        refuse(READ_FILE_ERROR_NULL_BUFFER);
        refuse(READ_FILE_ERROR_ZERO_SIZE);
        refuse(READ_FILE_ERROR_BUFFER_TOO_SMALL);
    }

    for (size_t i = 0; i < bytes_read; i += 1) {
        char c = buffer[i];
        if (c < 'a' || c > 'z') {
            *invalid_index = i;
            return VALIDATE_FILE_ERROR_INVALID;
        }
    }

    return error;
}

int main(int argc, char **argv) {
    size_t invalid_index = 0;

    if (argc < 2) {
        fprintf(stderr, "Please provide a text file to validate!\n");
        return 1;
    }

    switch (validate_file(argv[1], &invalid_index)) {
        ignore(READ_FILE_ERROR_NONE);

        case VALIDATE_FILE_ERROR_OPEN_FAILED:
            fprintf(stderr, "ERROR: Failed to open file with path %s!\n", argv[1]);
            return 1;
        case VALIDATE_FILE_ERROR_READ_FAILED:
            fprintf(stderr, "ERROR: Failed to read file!\n");
            return 1;
        case VALIDATE_FILE_ERROR_INVALID:
            fprintf(stderr, "ERROR: Invalid byte found at index %llu\n", invalid_index);
            return 1;
    }

    printf("File %s is valid!\n", argv[1]);
    return 0;
}
