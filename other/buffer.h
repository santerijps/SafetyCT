#ifndef SAFETYCT_BUFFER_H
#define SAFETYCT_BUFFER_H

#include <stdlib.h>
#include <string.h>

/**
 * @name BufferType
 * @brief The type of the buffer, static or dynamic.
 */
typedef enum buffer_type {
    BUFFER_TYPE_STATIC,     // The buffer capacity is static, and the buffer does not grow.
    BUFFER_TYPE_DYNAMIC,    // The buffer grows dynamically when needed.
} BufferType;

/**
 * @name Buffer
 * @brief A multi-purpose buffer for writing bytes.
 * The buffer can be used as a dynamic or static buffer.
 */
typedef struct buffer {
    BufferType type;        // Type of the buffer, either static or dynamic.
    size_t cap, len;        // Capacity (total size) and length (used size).
    unsigned char *ptr;     // Pointer to the underlying data.
} Buffer;

/**
 * @name BufferError
 * @brief An enum that contains all the buffer errors.
 */
typedef enum buffer_error {
    BUFFER_ERROR_NONE,          // No error.
    BUFFER_ERROR_NULL_BUFFER,   // The `buffer` pointer is null.
    BUFFER_ERROR_NULL_POINTER,  // The `pointer` to underlying data is null.
    BUFFER_ERROR_NULL_BYTES,    // The `bytes` pointer is null.
    BUFFER_ERROR_ZERO_CAPACITY, // The provided capacity is zero.
    BUFFER_ERROR_CALLOC_FAILED, // A call to `calloc` failed.
    BUFFER_ERROR_ZERO_COUNT,    // The specified count is zero.
    BUFFER_ERROR_CAPACITY_FULL, // The buffer capacity is full.
} BufferError;

/**
 * @name buffer_init_dynamic
 * @brief Initialize a dynamic buffer with a specified capacity.
 */
__attribute__((warn_unused_result)) BufferError buffer_init_dynamic(
    Buffer* const buffer,
    const size_t capacity
) {
    if (buffer == NULL) return BUFFER_ERROR_NULL_BUFFER;
    if (capacity == 0) return BUFFER_ERROR_ZERO_CAPACITY;

    buffer->type = BUFFER_TYPE_DYNAMIC;
    buffer->cap = capacity;
    buffer->len = 0;
    buffer->ptr = calloc(1, capacity);

    if (buffer->ptr == NULL) {
        return BUFFER_ERROR_CALLOC_FAILED;
    }

    return BUFFER_ERROR_NONE;
}

/**
 * @name buffer_init_static
 * @brief Initialize a static buffer with a specified capacity.
 */
BufferError buffer_init_static(
    Buffer* const buffer,
    void* const pointer,
    const size_t capacity
) {
    if (buffer == NULL) return BUFFER_ERROR_NULL_BUFFER;
    if (pointer == NULL) return BUFFER_ERROR_NULL_POINTER;
    if (capacity == 0) return BUFFER_ERROR_ZERO_CAPACITY;

    buffer->type = BUFFER_TYPE_STATIC;
    buffer->cap = capacity;
    buffer->len = 0;
    buffer->ptr = pointer;

    memset(pointer, 0, capacity);

    return BUFFER_ERROR_NONE;
}

/**
 * @name buffer_clear
 * @brief Clear the buffer of all of its contents and set `len` to 0.
 */
BufferError buffer_clear(Buffer* const buffer) {
    if (buffer == NULL) return BUFFER_ERROR_NULL_BUFFER;
    buffer->len = 0;
    memset(buffer->ptr, 0, buffer->cap);
    return BUFFER_ERROR_NONE;
}

/**
 * @name buffer_deinit
 * @brief Clear the buffer and free its memory if the buffer is dynamic.
 */
BufferError buffer_deinit(Buffer* const buffer) {
    if (buffer == NULL) return BUFFER_ERROR_NULL_BUFFER;
    BufferError error = buffer_clear(buffer);
    if (error != BUFFER_ERROR_NONE) return error;
    buffer_clear(buffer);
    if (buffer->type == BUFFER_TYPE_DYNAMIC) {
        free(buffer->ptr);
    }
    return BUFFER_ERROR_NONE;
}

/**
 * @name buffer_grow
 * @brief Grow the buffer to match the specified capacity.
 */
BufferError buffer_grow(
    Buffer* const buffer,
    const size_t capacity
) {
    if (buffer == NULL) return BUFFER_ERROR_NULL_BUFFER;
    if (buffer->type == BUFFER_TYPE_STATIC || buffer->cap > capacity) return BUFFER_ERROR_NONE;

    while (buffer->cap <= capacity) buffer->cap <<= 1;

    buffer->ptr = realloc(buffer->ptr, buffer->cap);
    if (buffer->ptr == NULL) return BUFFER_ERROR_CALLOC_FAILED;
    memset(buffer->ptr + buffer->len, 0, buffer->cap - buffer->len);

    return BUFFER_ERROR_NONE;
}

/**
 * @name buffer_write_byte
 * @brief Write a single byte into the buffer.
 */
BufferError buffer_write_byte(
    Buffer* const buffer,
    const unsigned char byte
) {
    if (buffer == NULL) return BUFFER_ERROR_NULL_BUFFER;

    if (buffer->type == BUFFER_TYPE_DYNAMIC) {
        BufferError error = buffer_grow(buffer, buffer->len + 1);
        if (error != BUFFER_ERROR_NONE) return error;

    } else if (buffer->len + 1 >= buffer->cap) {
        return BUFFER_ERROR_CAPACITY_FULL;
    }

    buffer->ptr[buffer->len] = byte;
    buffer->len += 1;

    return BUFFER_ERROR_NONE;
}

/**
 * @name buffer_write_bytes
 * @brief Write a specified amount of bytes into the buffer.
 */
BufferError buffer_write_bytes(
    Buffer* const buffer,
    const char* const bytes,
    const size_t count
) {
    if (buffer == NULL) return BUFFER_ERROR_NULL_BUFFER;
    if (bytes == NULL) return BUFFER_ERROR_NULL_BYTES;
    if (count == 0) return BUFFER_ERROR_ZERO_COUNT;

    if (buffer->type == BUFFER_TYPE_DYNAMIC) {
        BufferError error = buffer_grow(buffer, buffer->len + count);
        if (error != BUFFER_ERROR_NONE) return error;

    } else if (buffer->len + count >= buffer->cap) {
        return BUFFER_ERROR_CAPACITY_FULL;
    }

    for (size_t i = 0; i < count; i += 1) {
        buffer->ptr[buffer->len] = bytes[i];
        buffer->len += 1;
    }

    return BUFFER_ERROR_NONE;
}

/**
 * @name buffer_write_string
 * @brief Write a string into the buffer.
 */
BufferError buffer_write_string(
    Buffer* const buffer,
    const char* const string
) {
    size_t count = strlen(string);
    return buffer_write_bytes(buffer, string, count);
}

#endif
