#ifndef _UTILS_H
#define _UTILS_H

//Utility functions

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define STDOUT_FD 1

int fd_write_string(int fd, char *data);
void print_level_space(int level);
int stdout2fd_set(int fd);
int stdout2fd_reset(int stdout_copy_fd);
int string_is_float(char *str);
void system_usleep(unsigned long long us);
int system_random_int();
float system_random_float();

#endif