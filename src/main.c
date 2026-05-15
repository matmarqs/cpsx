#include "error.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define UNUSED(x) (void)(x)

#define BIOS_ADDR 0xbfc00000 // BIOS address in KSEG1
#define BIOS_SIZE (512 * 1024) // 512 KB

#define F_HEX32 "0x%08x"

uint8_t bios[BIOS_SIZE];

bool read_bios(const char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp) return false;

    fseek(fp, 0L, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    fread(bios, 1, size, fp);

    fclose(fp);
    return true;
}

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
    op_table_t *op_table;
};

void cpu_print_state(cpu_t *cpu)
{
    puts("==== CPU STATE ====");
    // Right-aligned PC to match the register width spacing
    printf("%5s = "F_HEX32"\n", "PC", cpu->pc);
    for (int i = 0; i < 32; i++) {
        char reg_name[8];
        snprintf(reg_name, sizeof(reg_name), "$%d", i);
        // %5s right-aligns the name string in a 5-character wide block
        printf("%5s = "F_HEX32"\n", reg_name, cpu->reg[i]);
    }
    puts("===================");
}

uint32_t inter_load32(inter_t *inter, uint32_t offset)
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

    uint32_t u32_instruction = (d << 24) | (c << 16) | (b << 8) | a;

    // little-endian: 0x12345678 in memory -> 0x78563412
    printf("Loaded "F_HEX32" with instruction "F_HEX32"\n", original_addr, u32_instruction);

    return u32_instruction;
}

uint32_t cpu_load32(cpu_t *cpu, uint32_t offset)
{
    return inter_load32(cpu->interconnect, offset);
}

uint32_t decode_instruction_opcode(instruction_t inst)
{
    // bits [31:26]
    return (inst.u32 >> 26);
}

uint32_t decode_instruction_reg(instruction_t inst)
{
    // bits [20:16]
    return (inst.u32 >> 16) & 0x1f;
}

uint32_t decode_instruction_imm(instruction_t inst)
{
    // bits [15:0]
    return inst.u32 & 0xff;
}

void op_unhandled(cpu_t *cpu, instruction_t inst)
{
    cpu_print_state(cpu);
    err_quit("Unhandled instruction: "F_HEX32, inst.u32);
}

void op_lui(cpu_t *cpu, instruction_t inst)
{
    uint32_t reg_index = decode_instruction_reg(inst);
    uint32_t value = decode_instruction_imm(inst);
    cpu->reg[reg_index] = (value << 16) & 0xffff0000;
    cpu->reg[0] = 0; // $zero is always 0
}

void cpu_main(cpu_t *cpu)
{
    while (true) {
        instruction_t instruction = { cpu_load32(cpu, cpu->pc) };

        uint32_t opcode = decode_instruction_opcode(instruction);
        cpu->op_table[opcode](cpu, instruction);
        cpu->pc += 4;
    }
}

int main(void)
{
    if (!read_bios("SCPH1001.BIN")) {
        err_quit("Failed to read BIOS");
        return 1;
    }

    inter_t interconnect;
    interconnect.bios = bios;

    cpu_t cpu;
    cpu.pc = BIOS_ADDR;
    for (int i = 0; i < 32; i++) {
        cpu.reg[i] = 0xdeadbeef; // initialize the registers with easy recognizable value
    }
    cpu.reg[0] = 0; // $zero should contain zero
    op_table_t op_table[64] = { 0 };
    for (int i = 0; i < 64; i++) {
        op_table[i] = op_unhandled;
    }
    op_table[15] = op_lui; // 001111 -> LUI (Load Upper Immediate)
    cpu.op_table = op_table;
    cpu.interconnect = &interconnect;

    cpu_main(&cpu);

    return 0;
}
