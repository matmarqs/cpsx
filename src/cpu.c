#include "cpu.h"
#include "interconnect.h"
#include "instruction.h"
#include "bios.h"
#include "op.h"

#include <stdio.h>
#include <stdbool.h>

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

uint32_t cpu_load32(cpu_t *cpu, uint32_t offset)
{
    return interconnect_load32(cpu->interconnect, offset);
}

bool cpu_store32(cpu_t *cpu, uint32_t offset, uint32_t value)
{
    return interconnect_store32(cpu->interconnect, offset, value);
}

void cpu_init(cpu_t *cpu, inter_t *interconnect)
{
    // initialize components
    interconnect_init(interconnect);
    cpu->interconnect = interconnect;

    // initialize registers
    cpu->pc = PSX_ADDR_BIOS;
    cpu->old_pc = 0;
    for (int i = 0; i < 32; i++) {
        cpu->reg[i] = 0xdeadbeef; // initialize the registers with easy recognizable value
    }
    cpu->reg[0] = 0; // $zero should contain zero
    cpu->next_instruction = (instruction_t) { .u32 = 0x0 }; // it's a NOP instruction

    // initialize global_optable
    op_init();
    cpu->op_table = global_optable;

    // initialize Coprocessor 0 registers
    for (int i = 0; i < 64; i++) {
        cpu->cop0.reg[i] = 0xdeadbeef;
    }
}

void cpu_main(cpu_t *cpu)
{
    while (true) {
        instruction_t instruction = cpu->next_instruction;
        cpu->next_instruction = (instruction_t) { cpu_load32(cpu, cpu->pc) }; // fetch at PC
        uint32_t opcode = decode_instruction_opcode(instruction);

        printf(F_HEX32": "F_HEX32"    ", cpu->old_pc, instruction.u32);
        cpu->old_pc = cpu->pc;

        cpu->pc += 4; // branch delay slot, this comes first than the execution
        cpu->op_table[opcode](cpu, instruction);
    }
}
