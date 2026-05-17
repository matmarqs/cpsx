#include "op.h"
#include "cpu.h"
#include "error.h"
#include "instruction.h"

#include <stdio.h>
#include <stdbool.h>

op_table_t global_optable[64];

static void init_optable(op_table_t *optable);

void op_init(void)
{
    static bool initialized = false;
    if (!initialized) {
        init_optable(global_optable);
    }
}

static void op_unhandled(cpu_t *cpu, instruction_t inst)
{
    puts("--unhandled--\n");
    cpu_print_state(cpu);

    uint32_t opcode = decode_instruction_opcode(inst);
    uint32_t rs = decode_instruction_rs(inst);
    uint32_t rt = decode_instruction_rt(inst);
    uint32_t imm = decode_instruction_imm(inst);

    err_quit("\nUnhandled instruction: "F_HEX32" = %06b %05b %05b %016b",
             inst.u32, opcode, rs, rt, imm);
}

static void op_lui(cpu_t *cpu, instruction_t inst)
{
    uint32_t reg_index = decode_instruction_rt(inst);
    uint32_t value = decode_instruction_imm(inst);
    cpu->reg[reg_index] = (value << 16) & 0xffff0000;
    cpu->reg[0] = 0; // $zero is always 0

    printf("lui $%d, 0x%x\n", reg_index, value);
}

static void op_ori(cpu_t *cpu, instruction_t inst)
{
    uint32_t rs = decode_instruction_rs(inst);
    uint32_t rt = decode_instruction_rt(inst);
    uint32_t imm = decode_instruction_imm(inst);
    cpu->reg[rt] = cpu->reg[rs] | imm;
    cpu->reg[0] = 0; // $zero is always 0

    printf("ori $%d, $%d, 0x%x\n", rt, rt, imm);
}

static void op_sw(cpu_t *cpu, instruction_t inst)
{
    // sw rt, offset(rs)
    uint32_t rs = decode_instruction_rs(inst);
    uint32_t rt = decode_instruction_rt(inst);
    uint32_t imm = decode_instruction_imm(inst);
    int16_t offset = (int16_t) imm; // 16-bit signed offset
    cpu_store32(cpu, cpu->reg[rs] + offset, cpu->reg[rt]);

    printf("sw $%d, 0x%x($%d)\n", rt, offset, rs);
}

static void init_optable(op_table_t *optable)
{
    for (int i = 0; i < 64; i++) {
        optable[i] = op_unhandled;
    }

    optable[13] = op_ori; // 13 = (001101)_2 -> ORI (Or Immediate)
    optable[15] = op_lui; // 15 = (001111)_2 -> LUI (Load Upper Immediate)
    optable[43] = op_sw;  // 43 = (101011)_2 -> SW (Store Word)
}
