#ifndef _IO_PORTS_H
#define _IO_PORTS_H

#include <stdint.h>

// KUSEG     KSEG0     KSEG1
// 1F801000h 9F801000h BF801000h  4K     I/O Ports
// region: 0x1f801000 <= x < 0x1f802000
// Hardware registers in memory
#define PSX_ADDR_MEMCONTROL 0x1F801000
#define PSX_SIZE_MEMCONTROL (4*1024)
extern uint8_t global_memcontrol[PSX_SIZE_MEMCONTROL];

// KUSEG     KSEG0     KSEG1
// 1F802000h 9F802000h BF802000h  8K     Expansion Region 2 (I/O Ports)
// region: 0x1f802000 <= x < 0x1f804000
// Expansion Region 2
#define PSX_ADDR_EXPREGION2 0x1F802000
#define PSX_SIZE_EXPREGION2 (8*1024)

extern uint8_t global_expregion2[PSX_SIZE_EXPREGION2];

// KUSEG     KSEG0     KSEG1
// 1FA00000h 9FA00000h BFA00000h  2048K  Expansion Region 3 (SRAM BIOS region for DTL cards)
// region: 0x1fa00000 <= x < 0x1fc00000
// Expansion Region 3 (SRAM BIOS region for DTL cards) -- This one is rarely used
#define PSX_ADDR_EXPREGION3 0x1FA00000
#define PSX_SIZE_EXPREGION3 (2048*1024)

#endif // _IO_PORTS_H
