#include <stdio.h>

#define STDIN 0
#define STDOUT 1

#define ASSERT(condition, message) {            \
    if (!(condition)) {                         \
        printf("%s \n", message);               \
        exit(-1);                               \
    }                                           \
}
