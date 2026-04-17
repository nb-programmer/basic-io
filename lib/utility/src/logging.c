#include <stdio.h>
#include <stdarg.h>

#include "utility/logging/logging.h"

unsigned int log_print_mask = -1;

void set_log_mask(unsigned int mask)
{
	log_print_mask = mask;
}

void lprintf(const char *tag, unsigned int level_mask, const char *format, ...)
{
	// Infinite arguments
	va_list args;
	va_start(args, format);
	if ((log_print_mask & level_mask) != 0)
	{
		printf("(%s) ", tag);
		// printf, but with variable arguments list
		vprintf(format, args);
	}
	va_end(args);
}
