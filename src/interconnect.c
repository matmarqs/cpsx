#include "interconnect.h"
#include "error.h"

// components
#include "bios.h"
#include "mmio.h"

#include <stdio.h>

static inline uint32_t addr_normalize(uint32_t addr)
{
    return addr & MASK_KUSEG;
}

static uint8_t* interconnect_resolve(inter_t *inter, uint32_t memloc, uint32_t *out_offset, bool write)
{
    // KSEG2: 0xC0000000 - 0xFFFFFFFF. Not masked by addr_normalize()
    if (memloc >= 0xC0000000) {
        // REGCONTROL is only in KSEG2
        if (PSX_ADDR_REGCONTROL <= memloc && memloc < PSX_ADDR_REGCONTROL + PSX_SIZE_REGCONTROL) {
            *out_offset = memloc - PSX_ADDR_REGCONTROL;
            return inter->regcontrol;
        }
        return NULL;
    }

    uint32_t addr = addr_normalize(memloc);

    if (addr < PSX_SIZE_MAINRAM) {
        *out_offset = addr;
        return inter->mainram;
    }
    //// EXPREGION1 not implemented yet
    //if (addr >= PSX_ADDR_EXPREGION1 && addr < PSX_ADDR_EXPREGION1 + PSX_SIZE_EXPREGION1) {
    //    *out_offset = addr - PSX_ADDR_EXPREGION1;
    //    return inter->expregion1;
    //}
    if (PSX_ADDR_MEMCONTROL <= addr && addr < PSX_ADDR_MEMCONTROL + PSX_SIZE_MEMCONTROL) {
        *out_offset = addr - PSX_ADDR_MEMCONTROL;
        return inter->memcontrol;
    }
    if (PSX_ADDR_EXPREGION2 <= addr && addr < PSX_ADDR_EXPREGION2 + PSX_SIZE_EXPREGION2) {
        *out_offset = addr - PSX_ADDR_EXPREGION2;
        return inter->expregion2;
    }
    if (PSX_ADDR_BIOS <= addr && addr < PSX_ADDR_BIOS + PSX_SIZE_BIOS) {
        if (write) {
            return NULL; // BIOS is read-only
        }
        *out_offset = addr - PSX_ADDR_BIOS;
        return inter->bios;
    }

    return NULL;
}

uint32_t interconnect_load32(inter_t *inter, uint32_t addr) {
    if (addr & MASK_02BITS) {
        err_debug("load32: Unaligned memory access: "F_HEX32, addr);
        return 0xdeadbeef;
    }

    uint32_t offset;
    uint8_t *target = interconnect_resolve(inter, addr, &offset, false);

    if (!target) {
        err_debug("load32: Unhandled memory address: "F_HEX32, addr);
        return 0xdeadbeef;
    }

    uint8_t *p = target + offset;
    return (p[3] << 24) | (p[2] << 16) | (p[1] << 8) | p[0];
}

bool interconnect_store32(inter_t *inter, uint32_t addr, uint32_t value) {
    if (addr & MASK_02BITS) {
        err_debug("store32: Unaligned memory access: "F_HEX32, addr);
        return false;
    }

    uint32_t offset;
    uint8_t *target = interconnect_resolve(inter, addr, &offset, true);

    if (!target) {
        err_debug("store32: Unhandled memory address: "F_HEX32, addr);
        return false;
    }

    *(uint32_t*)(target + offset) = value;
    return true;
}

bool interconnect_store16(inter_t *inter, uint32_t addr, uint16_t value)
{
    if (addr & MASK_01BITS) {
        err_debug("store16: Unaligned memory address: "F_HEX32, addr);
        return false;
    }
    uint32_t offset;
    uint8_t *target = interconnect_resolve(inter, addr, &offset, true);
    if (!target) {
        err_debug("store16: Unhandled memory address "F_HEX32, addr);
    }
    *(uint16_t *)(target + offset) = value;
    return true;
}

bool interconnect_store8(inter_t *inter, uint32_t addr, uint8_t value)
{
    uint32_t offset;
    uint8_t *target = interconnect_resolve(inter, addr, &offset, true);
    if (!target) {
        err_debug("store16: Unhandled memory address "F_HEX32, addr);
    }
    *(uint8_t *)(target + offset) = value;
    return true;
}

uint8_t interconnect_load8(inter_t *inter, uint32_t addr)
{
    uint32_t offset;
    uint8_t *target = interconnect_resolve(inter, addr, &offset, false);

    if (!target) {
        err_debug("load32: Unhandled memory address: "F_HEX32, addr);
        return 0xff; // doesn't matter what we return
    }

    uint8_t *p = target + offset;
    return p[0];
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
    inter->mainram = global_mainram;
}
