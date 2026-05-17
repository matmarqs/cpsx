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
    uint8_t *io_ports1; // I/O Ports, 4K
    uint8_t *io_ports2; // Expansion Region 2, 8K
} inter_t;

typedef struct {
    uint32_t u32;
} instruction_t;

typedef struct cpu_t cpu_t;

typedef void (*op_table_t)(cpu_t *, instruction_t);

struct cpu_t {
    uint32_t pc; // program counter
    uint32_t reg[32]; // general purpose registers
    inter_t *interconnect; // struct that holds all other components
    op_table_t *op_table; // table with 64 entries for each MIPS instruction
};

#define F_HEX32 "0x%08x"
#define UNUSED(x) (void)(x)

#endif // _TYPES_H
