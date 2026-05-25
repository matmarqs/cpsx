#include "interconnect.h"
#include "error.h"

// components
#include "bios.h"
#include "io_ports.h"

#include <stdio.h>

uint32_t interconnect_load32(inter_t *inter, uint32_t offset)
{
    if ((offset & MASK_02BITS) != 0) { // test if multiple of 4
        err_debug("store32: Unaligned memory access: "F_HEX32, offset);
    }

    uint8_t *target = NULL;

    if (PSX_ADDR_BIOS <= offset && offset < PSX_ADDR_BIOS + PSX_SIZE_BIOS) {
        target = inter->bios;
        offset -= PSX_ADDR_BIOS;
    }

    if (!target) {
        err_debug("load32: Unhandled memory address: "F_HEX32, offset);
    }

    uint8_t a = target[offset + 0];
    uint8_t b = target[offset + 1];
    uint8_t c = target[offset + 2];
    uint8_t d = target[offset + 3];

    // little-endian: 0x12345678 in memory -> 0x78563412
    uint32_t u32_instruction = (d << 24) | (c << 16) | (b << 8) | a;

    return u32_instruction;
}

bool interconnect_store32(inter_t *inter, uint32_t offset, uint32_t value)
{
    if ((offset & MASK_02BITS) != 0) { // test if multiple of 4
        err_debug("store32: Unaligned memory access: "F_HEX32, offset);
    }

    uint32_t *target;

    if (PSX_ADDR_MEMCONTROL <= offset && offset < PSX_ADDR_MEMCONTROL + PSX_SIZE_MEMCONTROL) {
        // for now we will not emulate low-level stuff related to these I/O Ports (HW registers)
        target = (uint32_t *) (inter->memcontrol + offset - PSX_ADDR_MEMCONTROL);
    }
    else if (PSX_ADDR_EXPREGION2 <= offset && offset < PSX_ADDR_EXPREGION2 + PSX_SIZE_EXPREGION2) {
        target = (uint32_t *) (inter->expregion2 + offset - PSX_ADDR_EXPREGION2);
    }
    else if (PSX_ADDR_REGCONTROL <= offset && offset < PSX_ADDR_REGCONTROL + PSX_SIZE_REGCONTROL) {
        target = (uint32_t *) (inter->regcontrol + offset - PSX_ADDR_REGCONTROL);
    }
    else {
        err_debug("store32: Unhandled memory address: "F_HEX32, offset);
        return false;
    }

    *target = value;

    return true;
}

void interconnect_init(inter_t *inter)
{
    if (!read_bios("SCPH1001.BIN")) {
        err_quit("Failed to read BIOS 'SCPH1001.BIN' at current directory");
    }
    inter->bios = global_bios;
    inter->memcontrol = global_memcontrol;
    inter->expregion2 = global_expregion2;
    inter->regcontrol = global_regcontrol;
}
