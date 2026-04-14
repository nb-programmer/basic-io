#include "platform.h"

#ifdef __linux__
#include <unistd.h>

#define STDOUT_FD 1

// Utility function to write a C string to a Linux file
int fd_write_string(int fd, char *data) {
    return write(fd, data, strlen(data));
}

// Hey, this actually came into use!
int stdout2fd_set(int fd) {
    // Create a copy of stdout to restore later
    int stdout_copy_fd = dup(STDOUT_FD);

    // Rename stdout to that of the given fd
    if (dup2(fd, STDOUT_FD) < 0) {
		fprintf(stderr, "Failed to make stdout redirection\n");
		return -1;
	}

    return stdout_copy_fd;
}

int stdout2fd_reset(int stdout_copy_fd) {
    // Rename stdout to that of the given fd, which should be the original stdout file
    if (dup2(stdout_copy_fd, STDOUT_FD) < 0) {
		fprintf(stderr, "Failed to restore stdout redirection\n");
		return -1;
	}
    return STDOUT_FD;
}
#endif
