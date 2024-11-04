#ifndef SAFETYCT_SLICE_H
#define SAFETYCT_SLICE_H

#define SLICE(pointer, start, stop) (Slice) {.ptr = ((pointer) + (start)), .len = (stop) - (start)}
#define SLICE_CAST(slice, type) ((type)((slice).ptr))

// To write a slice as a string using any of the printf functions,
// use the %.*s format specifier, and use slice.len and SLICE_CAST(slice, char*) as arguments.

#define SLICE_CMP(slice, pointer, type)\
    ({\
        int equal = 1;\
        for (int i = 0; i < (slice).len; i += 1) {\
            if (SLICE_CAST(slice, type)[i] != ((type)(pointer))[i]) {\
                equal = 0;\
                break;\
            }\
        }\
        equal;\
    })

typedef struct slice {
    void *ptr;
    int len;
} Slice;

#endif
