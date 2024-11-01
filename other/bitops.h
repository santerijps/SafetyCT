#ifndef SAFETYCT_BITOPS_H
#define SAFETYCT_BITOPS_H

#define BIT_PRINT(x)                                                \
    for (unsigned long long i = 0; i < sizeof(x) * 8; i += 1) {     \
        typeof(x) y = 1 << (sizeof(x) * 8 - 1 - i);                 \
        putc(!!(x & y) + 48, stdout);                               \
        if (i + 1 == sizeof(x) * 8) {                               \
            putc('\n', stdout);                                     \
        }                                                           \
    }

#define BIT_SIZE(x) (sizeof(x) * 8)

#define BIT_SET_LEFT(x, n) ((x) | (1 << (BIT_SIZE(x) - (n))))
#define BIT_SET_RIGHT(x, n) ((x) | (1 << ((n) - 1)))

#define BIT_CLEAR_LEFT(x, n) ((x) & ~(1 << (BIT_SIZE(x) - (n))))
#define BIT_CLEAR_RIGHT(x, n) ((x) & ~(1 << ((n) - 1)))

#define BIT_TOGGLE_LEFT(x, n) ((x) ^ (1 << (BIT_SIZE(x) - (n))))
#define BIT_TOGGLE_RIGHT(x, n) ((x) ^ (1 << ((n) - 1)))

#define BIT_GET_LEFT(x, n) (((x) >> (BIT_SIZE(x) - n)) & 1)
#define BIT_GET_RIGHT(x, n) ((x) >> (n) - 1 & 1)

#endif
