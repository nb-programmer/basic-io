#include <stdio.h>
#include <stdlib.h>

#ifdef __linux__
#include <unistd.h>
#elif defined(_WIN32) || defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

void system_sleep(float seconds)
{
#ifdef __linux__
	// Calculate the duration in terms of microseconds (x 10^6)
	unsigned long long duration = (unsigned long long)(seconds * 1e6);
	usleep(duration);
#elif defined(_WIN32) || defined(_WIN64)
	// Calculate the duration in terms of milliseconds (x 10^3)
	unsigned long duration = (unsigned long long)(seconds * 1e3);
	Sleep(duration);
#else
#error "Found no system-specific implementation for system_sleep. It will be stubbed."
#endif
}

int system_random_int()
{
	// Generate a random number between 0 and RAND_MAX range
	return rand();
}

float system_random_float()
{
	// Generate a random number between 0 and 1
	return ((float)rand()) / (float)RAND_MAX;
}
