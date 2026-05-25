#ifndef _UTIL_H
#define _UTIL_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

bool read_file(const char *filename, void *buffer, size_t buffer_size);
bool detect_overflow_i32(int32_t a, int32_t b);

#endif // _UTIL_H
