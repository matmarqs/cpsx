#ifndef _BIOS_H
#define _BIOS_H

#include <stdint.h>
#include <stdbool.h>

#define PSX_ADDR_BIOS 0xbfc00000 // BIOS address in KSEG1
#define PSX_SIZE_BIOS (512 * 1024) // 512 KB

extern uint8_t global_bios[PSX_SIZE_BIOS];

bool read_bios(const char *filename);

#endif // _BIOS_H
