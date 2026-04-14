
#include "utils.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Prints 'level' number of "tab" characters
void print_level_space(int level) {
    for (int i=0;i<level;i++) printf("    ");
}
