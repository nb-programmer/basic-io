#pragma once

int stdout2fd_set(int fd);
int stdout2fd_reset(int stdout_copy_fd);
int fd_write_string(int fd, char *data);
