#ifndef _TYPES_H
#define _TYPES_H

#include <stdint.h>

typedef struct {
    uint8_t *bios; // BIOS ROM
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
