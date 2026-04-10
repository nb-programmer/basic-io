#include <string.h>

int string_is_float(char *str) {
    while (*str != '\0')
        if (*str++ == '.')
            return 1;
    return 0;
}
