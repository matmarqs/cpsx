#include "interconnect.h"
#include "error.h"
#include "bios.h"

#include <stdio.h>

uint32_t interconnect_load32(inter_t *inter, uint32_t offset)
{
    if ((offset & 0x3) != 0) { // test if multiple of 4
        err_quit("Unaligned memory access: "F_HEX32, offset);
    }

    uint32_t original_addr = offset;

    uint8_t *target;

    if (BIOS_ADDR <= offset && offset < BIOS_ADDR + BIOS_SIZE) {
        target = inter->bios;
        offset -= BIOS_ADDR;
    }
    else {
        err_quit("Unhandled memory address: "F_HEX32, offset);
    }

    uint8_t a = target[offset + 0];
    uint8_t b = target[offset + 1];
    uint8_t c = target[offset + 2];
    uint8_t d = target[offset + 3];

    // little-endian: 0x12345678 in memory -> 0x78563412
    uint32_t u32_instruction = (d << 24) | (c << 16) | (b << 8) | a;
    printf(F_HEX32": "F_HEX32"    ", original_addr, u32_instruction);

    return u32_instruction;
}

// TODO: actually write store32
uint32_t interconnect_store32(inter_t *inter, uint32_t offset, uint32_t value)
{
    if ((offset & 0x3) != 0) { // test if multiple of 4
        err_quit("Unaligned memory access: "F_HEX32, offset);
    }

    return 0;
}


void interconnect_init(inter_t *inter)
{
    if (!read_bios("SCPH1001.BIN")) {
        err_quit("Failed to read BIOS 'SCPH1001.BIN' at current directory");
    }
    inter->bios = global_bios;
}
