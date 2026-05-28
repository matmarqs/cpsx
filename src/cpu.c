#include "cpu.h"
#include "interconnect.h"
#include "instruction.h"
#include "bios.h"
#include "op.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

void cpu_print_state(cpu_t *cpu)
{
    puts("==== CPU STATE ====");
    // Right-aligned PC to match the register width spacing
    printf("%5s = "F_HEX32"\n", "PC", cpu->pc);
    for (int i = 0; i < 32; i++) {
        char reg_name[8];
        snprintf(reg_name, sizeof(reg_name), "$%d", i);
        // %5s right-aligns the name string in a 5-character wide block
        printf("%5s = "F_HEX32"\n", reg_name, cpu_reg(cpu, i));
    }
    puts("===================");
}

uint32_t cpu_load32(cpu_t *cpu, uint32_t mem_location)
{
    return interconnect_load32(cpu->interconnect, mem_location);
}

bool cpu_store32(cpu_t *cpu, uint32_t offset, uint32_t value)
{
    return interconnect_store32(cpu->interconnect, offset, value);
}

bool cpu_store16(cpu_t *cpu, uint32_t offset, uint16_t value)
{
    return interconnect_store16(cpu->interconnect, offset, value);
}

bool cpu_store8(cpu_t *cpu, uint32_t offset, uint8_t value)
{
    return interconnect_store8(cpu->interconnect, offset, value);
}

void cpu_init(cpu_t *cpu, inter_t *interconnect)
{
    // initialize components
    interconnect_init(interconnect);
    cpu->interconnect = interconnect;

    // initialize registers
    cpu->pc = PSX_ADDR_BIOS_1; // 0xBFC00000
    cpu->old_pc = 0;

    // initialize register banks
    for (int i = 0; i < 32; i++) {
        cpu->regs[i] = 0xdeadbeef; // initialize the registers with easy recognizable value
        cpu->out_regs[i] = 0xdeadbeef;
    }
    cpu->regs[0] = 0; // $zero should contain zero
    cpu->out_regs[0] = 0;

    cpu->next_instruction = (instruction_t) { .u32 = 0x0 }; // it's a NOP instruction
    cpu->load = (load_set_t) { .reg_index = 0, .value = 0 };

    // initialize global_optable
    op_init();
    cpu->op_table = global_optable;

    // initialize Coprocessor 0 registers
    for (int i = 0; i < 64; i++) {
        cpu->cop0.regs[i] = 0xdeadbeef;
    }
    cpu->cop0.regs[12] = 0; // $cop0_12 is the status register
}

/* we assume 0 <= index < 32 */
inline uint32_t cpu_reg(cpu_t *cpu, uint8_t index)
{
    return cpu->regs[index];
}

inline void cpu_set_reg(cpu_t *cpu, uint8_t index, uint32_t value)
{
    cpu->out_regs[index] = value;
    cpu->out_regs[0] = 0; // $zero is always zero
}

inline void cpu_load_delay(cpu_t *cpu, uint8_t reg_index, uint32_t value)
{
    cpu->load.reg_index = reg_index;
    cpu->load.value = value;
}

void cpu_main(cpu_t *cpu)
{
    while (true) {
        instruction_t instruction = cpu->next_instruction;
        cpu->next_instruction = (instruction_t) { cpu_load32(cpu, cpu->pc) }; // fetch at PC
        uint32_t opcode = decode_instruction_opcode(instruction);

        printf(F_HEX32": "F_HEX32"    ", cpu->old_pc, instruction.u32);
        cpu->old_pc = cpu->pc;

        // Step 1: Apply pending load to NEXT registers and then reset the load
        // This writes to the output bank, NOT visible to current instruction
        cpu_set_reg(cpu, cpu->load.reg_index, cpu->load.value);
        cpu->load = (load_set_t) { .reg_index = 0, .value = 0 };

        // Step 2: Execute instruction (read from regs, write to out_regs)
        cpu->pc += 4; // branch delay slot, this comes first than the execution
        cpu->op_table[opcode](cpu, instruction);

        // Step 3: Copy out_regs to regs
        memcpy(cpu->regs, cpu->out_regs, sizeof(cpu->regs)); // load delays, we will optimize it later
    }
}
