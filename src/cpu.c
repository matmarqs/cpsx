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

void cpu_init(cpu_t *cpu, inter_t *interconnect)
{
    // initialize components
    interconnect_init(interconnect);
    cpu->interconnect = interconnect;

    // initialize registers
    cpu->pc = BIOS_ADDR;
    for (int i = 0; i < 32; i++) {
        cpu->reg[i] = 0xdeadbeef; // initialize the registers with easy recognizable value
    }
    cpu->reg[0] = 0; // $zero should contain zero

    // initialize global_optable
    op_init();
    cpu->op_table = global_optable;
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
