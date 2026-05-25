#ifndef _BIOS_H
#define _BIOS_H

#include "mmio.h"

#include <stdbool.h>

bool read_bios(const char *filename);

#endif // _BIOS_H
