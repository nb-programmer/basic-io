#pragma once

void system_sleep(unsigned long long us);
int system_random_int();
float system_random_float();
void system_tty_write(const char *str);
void system_tty_printf(const char *format, ...);
void system_tty_flush_output();
