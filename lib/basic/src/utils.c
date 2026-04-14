#include <string.h>
#include <stdio.h>

// Prints 'level' number of "tab" characters
void print_level_space(int level)
{
	for (int i = 0; i < level; i++)
		printf("    ");
}

int string_is_float(char *str)
{
	while (*str != '\0')
		if (*str++ == '.')
			return 1;
	return 0;
}
