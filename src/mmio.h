#ifndef _MMIO_H
#define _MMIO_H

#include <stdint.h>

// Mask to KUSEG (clear upper 3 bits of address)
#define MASK_KUSEG 0x1FFFFFFF

// KUSEG     KSEG0     KSEG1
// 1F000000h 9F000000h BF000000h  8192K  Expansion Region 1 (ROM/RAM)
// region: 0x1f000000 <= x < 0x1f800000
// ROM and RAM. There is also Expansion 1 Header.
#define PSX_ADDR_EXPREGION1   0x1F000000
#define PSX_SIZE_EXPREGION1   (8192*1024)
extern uint8_t global_expregion1[PSX_SIZE_EXPREGION1];
void expregion1_init(void);

// KUSEG     KSEG0     KSEG1
// 1F801000h 9F801000h BF801000h  4K     I/O Ports
// region: 0x1f801000 <= x < 0x1f802000
// Hardware registers in memory
#define PSX_ADDR_MEMCONTROL   0x1F801000
//#define PSX_ADDR_MEMCONTROL_0 0x9F801000
//#define PSX_ADDR_MEMCONTROL_1 0xBF801000
#define PSX_SIZE_MEMCONTROL   (4*1024)  // 4 KB
extern uint8_t global_memcontrol[PSX_SIZE_MEMCONTROL];

// KUSEG     KSEG0     KSEG1
// 1F802000h 9F802000h BF802000h  8K     Expansion Region 2 (I/O Ports)
// region: 0x1f802000 <= x < 0x1f804000
// Expansion Region 2
#define PSX_ADDR_EXPREGION2   0x1F802000
//#define PSX_ADDR_EXPREGION2_0 0x9F802000
//#define PSX_ADDR_EXPREGION2_1 0xBF802000
#define PSX_SIZE_EXPREGION2   (8*1024) // 8 KB
extern uint8_t global_expregion2[PSX_SIZE_EXPREGION2];

// KUSEG     KSEG0     KSEG1
// 1FA00000h 9FA00000h BFA00000h  2048K  Expansion Region 3 (SRAM BIOS region for DTL cards)
// region: 0x1fa00000 <= x < 0x1fc00000
// Expansion Region 3 (SRAM BIOS region for DTL cards) -- This one is rarely used
#define PSX_ADDR_EXPREGION3   0x1FA00000
//#define PSX_ADDR_EXPREGION3_0 0x9FA00000
//#define PSX_ADDR_EXPREGION3_1 0xBFA00000
#define PSX_SIZE_EXPREGION3   (2048*1024) // 2048 KB, 2 MB

// FFFE0000h (in KSEG2)     0.5K   Internal CPU control registers (Cache Control)
// region: 0xfffe0000 <= x < 0xfffe0200
// Cache Control register is 0xfffe0130 (inside this region)
#define PSX_ADDR_REGCONTROL 0xFFFE0000
#define PSX_SIZE_REGCONTROL 512 // 0.5 KB
extern uint8_t global_regcontrol[PSX_SIZE_REGCONTROL];

// KUSEG     KSEG0     KSEG1
// 00000000h 80000000h A0000000h  2048K  Main RAM (first 64K reserved for BIOS)
// region: 0x00000000 <= x < 0x00200000
// Main RAM
#define PSX_ADDR_MAINRAM   0x00000000
//#define PSX_ADDR_MAINRAM_0 0x80000000
//#define PSX_ADDR_MAINRAM_1 0xA0000000
#define PSX_SIZE_MAINRAM   (2048*1024) // 2048 KB, 2 MB
extern uint8_t global_mainram[PSX_SIZE_MAINRAM];

// KUSEG     KSEG0     KSEG1
// 1FC00000h 9FC00000h BFC00000h  512K   BIOS ROM (Kernel) (4096K max)
// region: 0x1fc00000 <= x < 0x1fc80000
// BIOS region
#define PSX_ADDR_BIOS   0x1fc00000 // BIOS address in KSEG
//#define PSX_ADDR_BIOS_0 0x9fc00000 // BIOS address in KSEG0
#define PSX_ADDR_BIOS_1 0xbfc00000 // BIOS address in KSEG1, entry point for the BIOS
#define PSX_SIZE_BIOS   (512*1024) // 512 KB
extern uint8_t global_bios[PSX_SIZE_BIOS];

#endif // _MMIO_H
