#include "interconnect.h"
#include "error.h"

// components
#include "bios.h"
#include "io_ports.h"

#include <stdio.h>

uint32_t interconnect_load32(inter_t *inter, uint32_t offset)
{
    if ((offset & 0x3) != 0) { // test if multiple of 4
        err_quit("store32: Unaligned memory access: "F_HEX32, offset);
    }

    uint32_t original_addr = offset;

    uint8_t *target;

    if (BIOS_ADDR <= offset && offset < BIOS_ADDR + BIOS_SIZE) {
        target = inter->bios;
        offset -= BIOS_ADDR;
    }
    else {
        err_quit("load32: Unhandled memory address: "F_HEX32, offset);
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
bool interconnect_store32(inter_t *inter, uint32_t offset, uint32_t value)
{
    if ((offset & 0x3) != 0) { // test if multiple of 4
        err_quit("store32: Unaligned memory access: "F_HEX32, offset);
    }

    uint32_t *target;

    if (IO_PORTS1_ADDR <= offset && offset < IO_PORTS1_ADDR + IO_PORTS1_SIZE) {
        // for now we will not emulate low-level stuff related to these I/O Ports (HW registers)
        target = (uint32_t *) (inter->io_ports1 + offset - IO_PORTS1_ADDR);
    }
    else if (IO_PORTS2_ADDR <= offset && offset < IO_PORTS2_ADDR + IO_PORTS2_SIZE) {
        target = (uint32_t *) (inter->io_ports2 + offset - IO_PORTS2_ADDR);
    }
    else {
        err_quit("store32: Unhandled memory address: "F_HEX32, offset);
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
    inter->io_ports1 = global_io_ports_1;
    inter->io_ports2 = global_io_ports_2;
}
