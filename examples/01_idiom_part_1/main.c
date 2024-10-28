#include "../../safetyct.h"

enum ReadFileError {
    READ_FILE_ERROR_NONE,
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
    presume(file_path != NULL);
    presume(buffer != NULL);
    presume(size > 0);

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

int main(int argc, char **argv) {
    char buffer[16] = {0};
    size_t bytes_read = 0;

    if (argc < 2) {
        fprintf(stderr, "Please provide a text file to read!\n");
        return 1;
    }

    switch (read_file(argv[1], buffer, len(buffer), &bytes_read)) {
        case READ_FILE_ERROR_NONE:
            break;
        case READ_FILE_ERROR_OPEN_FAILED:
            fprintf(stderr, "ERROR: Failed to open file with path %s!\n", argv[1]);
            return 1;
        case READ_FILE_ERROR_READ_FAILED:
            fprintf(stderr, "ERROR: Failed to read file!\n");
            return 1;
        case READ_FILE_ERROR_BUFFER_TOO_SMALL:
            fprintf(stderr, "ERROR: Buffer size of %llu is not enough!\n", len(buffer));
            return 1;
    }

    printf("Read %llu bytes: %s\n", bytes_read, buffer);
    return 0;
}
