#ifndef _TYPES_H
#define _TYPES_H

#include <stdint.h>
#include <stdbool.h>

//  KUSEG     KSEG0     KSEG1
//  00000000h 80000000h A0000000h  2048K  Main RAM (first 64K reserved for BIOS)
//  1F000000h 9F000000h BF000000h  8192K  Expansion Region 1 (ROM/RAM)
//  1F800000h 9F800000h    --      1K     Scratchpad (D-Cache used as Fast RAM)
//  1F801000h 9F801000h BF801000h  4K     I/O Ports
//  1F802000h 9F802000h BF802000h  8K     Expansion Region 2 (I/O Ports)
//  1FA00000h 9FA00000h BFA00000h  2048K  Expansion Region 3 (SRAM BIOS region for DTL cards)
//  1FC00000h 9FC00000h BFC00000h  512K   BIOS ROM (Kernel) (4096K max)
//        FFFE0000h (in KSEG2)     0.5K   Internal CPU control registers (Cache Control)
typedef struct {
    uint8_t *bios; // BIOS ROM, 512 K
    uint8_t *memcontrol; // I/O Ports, 4K
    uint8_t *expregion2; // Expansion Region 2, 8K
} inter_t;

typedef struct {
    uint32_t u32;
} instruction_t;

typedef struct cpu_t cpu_t;

typedef void (*op_table_t)(cpu_t *, instruction_t);

struct cpu_t {
    uint32_t pc; // program counter
    uint32_t old_pc;
    uint32_t reg[32]; // general purpose registers
    instruction_t next_instruction; // needed to emulate the branch delay slot of MIPS
    inter_t *interconnect; // struct that holds all other components
    op_table_t *op_table; // table with 64 entries for each MIPS instruction
};

#define F_HEX32 "0x%08x"
#define UNUSED(x) (void)(x)

// constant for masking bits. the formula is: hex((1 << n) - 1)
#define MASK_02BITS 0x3     // 0x3 = 0b11
#define MASK_05BITS 0x1f    // 0x1f = 0b11111
#define MASK_06BITS 0x3f    // 0x3f = 0b111111
#define MASK_16BITS 0xffff  // 0xffff = 0b1111111111111111
#define MASK_26BITS 0x3ffffff // 0x3ffffff = 0b11111111111111111111111111

#endif // _TYPES_H
