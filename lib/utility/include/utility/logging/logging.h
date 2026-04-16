#pragma once


// Log level mask
#define LOGTYPE_DEBUG (1 << 0)
#define LOGTYPE_INFO (1 << 1)
#define LOGTYPE_MESSAGE (1 << 2)
#define LOGTYPE_ERROR (1 << 3)
#define LOGMASK_ALL 0xffff

/**
 * Set logger mask.
 * 
 * ## Parameters
 * - `mask`: log mask. One or multiple (using bitwise or) of the following:
 * (`LOGTYPE_DEBUG`, `LOGTYPE_INFO`, `LOGTYPE_MESSAGE`, `LOGTYPE_ERROR`)
 * 
*/
void set_log_mask(unsigned int mask);

// printf modified to allow only selected messages
void lprintf(const char *tag, unsigned int level_mask, const char *format, ...);
