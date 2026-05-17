#ifndef _IO_PORTS_H
#define _IO_PORTS_H

#include <stdint.h>

// KUSEG     KSEG0     KSEG1
// 1F801000h 9F801000h BF801000h  8K     I/O Ports

// region: 0x1f801000 -- 0x1f803000

// I/O Ports
#define PSX_ADDR_MEMCONTROL 0x1F801000
#define PSX_SIZE_MEMCONTROL (4*1024)

extern uint8_t global_memcontrol[PSX_SIZE_MEMCONTROL];

// Expansion Region 2
#define PSX_ADDR_EXPREGION2 0x1F802000
#define PSX_SIZE_EXPREGION2 (8*1024)

extern uint8_t global_expregion2[PSX_SIZE_EXPREGION2];

// Expansion Region 3 (SRAM BIOS region for DTL cards)
#define PSX_ADDR_EXPREGION3 0x1FA00000
#define PSX_SIZE_EXPREGION3 (2048*1024)

#endif // _IO_PORTS_H
