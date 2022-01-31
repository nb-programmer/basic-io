
#include "utils.h"

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//Utility function to write a C string to a Linux file
int fd_write_string(int fd, char *data) {
    return write(fd, data, strlen(data));
}

//Prints 'level' number of "tab" characters
void print_level_space(int level) {
    for (int i=0;i<level;i++) printf("    ");
}

//Hey, this actually came into use!
int stdout2fd_set(int fd) {
    //Create a copy of stdout to restore later
    int stdout_copy_fd = dup(STDOUT_FD);

    //Rename stdout to that of the given fd
    if (dup2(fd, STDOUT_FD) < 0) {
		fprintf(stderr, "Failed to make stdout redirection\n");
		return -1;
	}

    return stdout_copy_fd;
}

int stdout2fd_reset(int stdout_copy_fd) {
    //Rename stdout to that of the given fd, which should be the original stdout file
    if (dup2(stdout_copy_fd, STDOUT_FD) < 0) {
		fprintf(stderr, "Failed to restore stdout redirection\n");
		return -1;
	}
    return STDOUT_FD;
}

int string_is_float(char *str) {
    while (*str != '\0')
        if (*str++ == '.')
            return 1;
    return 0;
}

void system_usleep(unsigned long long us) {
    usleep(us);
}

int system_random_int() {
    //Generate a random number between 0 and RAND_MAX range
    return rand();
}

float system_random_float() {
    //Generate a random number between 0 and 1
    return ((float)rand()) / (float)RAND_MAX;
}