#ifndef _BIOS_H
#define _BIOS_H

#include <stdint.h>
#include <stdbool.h>

#define BIOS_ADDR 0xbfc00000 // BIOS address in KSEG1
#define BIOS_SIZE (512 * 1024) // 512 KB

extern uint8_t global_bios[BIOS_SIZE];

bool read_bios(const char *filename);

#endif // _BIOS_H
