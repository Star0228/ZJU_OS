#include "string.h"
#include "stdint.h"

void *memset(void *dest, int c, uint64_t n) {
    char *s = (char *)dest;
    for (uint64_t i = 0; i < n; ++i) {
        s[i] = c;
    }
    return dest;
}

//added in Lab4
void *memcpy(void *dest, void *src, uint64_t n) {
    char *d = dest;
    char *s = src;
    while (n--) {
        *(d++) = *(s++);
    }
    return dest;
}
//end added in Lab4

int memcmp(const void *s1, const void *s2, uint64_t n) {
    const unsigned char *c1 = s1, *c2 = s2;
    while (n-- > 0) {
        if (*c1 != *c2) {
            return *c1 - *c2;
        }
        c1++;
        c2++;
    }
    return 0;
}

int strlen(const char *s) {
    int len = 0;
    while (s[len]) {
        len++;
    }
    return len;
}