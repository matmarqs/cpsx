#include "op.h"
#include "cpu.h"
#include "error.h"
#include "instruction.h"
#include "util.h"

#include <stdio.h>
#include <stdbool.h>

op_table_t global_optable[64];
op_table_t global_special_optable[64];
op_table_t global_cop0_optable[32];

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

    err_debug("\nUnhandled instruction: "F_HEX32" = %06b %05b %05b %016b",
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

    printf("ori $%d, $%d, 0x%x\n", rt, rs, imm);
}

static void op_sw(cpu_t *cpu, instruction_t inst)
{
    // sw rt, offset(rs)
    uint32_t base = decode_instruction_rs(inst);
    uint32_t rt = decode_instruction_rt(inst);
    int16_t offset = (int16_t) decode_instruction_imm(inst);; // 16-bit signed offset

    if ((cpu->cop0.reg[12] & 0x10000) != 0) { // $cop0_12 is the status register
        // Cache is isolated, ignore write
        printf("sw $%d, 0x%x($%d)\t;; but ignoring store while cache is isolated\n", rt, offset, base);
        return;
    }
    printf("sw $%d, 0x%x($%d)\n", rt, offset, base);
    cpu_store32(cpu, cpu->reg[base] + offset, cpu->reg[rt]);
}

static void op_special(cpu_t *cpu, instruction_t inst)
{
    uint32_t special_op = decode_instruction_special_op(inst);
    global_special_optable[special_op](cpu, inst);
}

static void op_sll(cpu_t *cpu, instruction_t inst)
{
    // sw rt, offset(rs)
    uint32_t rt = decode_instruction_rt(inst);
    uint32_t rd = decode_instruction_rd(inst);
    uint32_t sa = decode_instruction_sa(inst);
    cpu->reg[rd] = cpu->reg[rt] << sa;
    cpu->reg[0] = 0; // $zero is always 0

    if (!rt && !rd && !sa) { // sll $0, $0, 0 is a nop instruction
        printf("nop\n");
    }
    else {
        printf("sll $%d, $%d, $%d\n", rd, rt, sa);
    }
}

static void op_addiu(cpu_t *cpu, instruction_t inst)
{
    // The 16-bit signed immediate is added to the 32-bit value in GPR rs and the 32-bit arithmetic result is placed into GPR rt.
    // No Integer Overflow exception occurs under any circumstances.
    uint32_t rs = decode_instruction_rs(inst);
    uint32_t rt = decode_instruction_rt(inst);
    int16_t imm = (int16_t) decode_instruction_imm(inst);
    cpu->reg[rt] = cpu->reg[rs] + imm;
    cpu->reg[0] = 0; // $zero is always 0

    printf("addiu $%d, $%d, 0x%x\n", rt, rs, imm);
}

static void op_j(cpu_t *cpu, instruction_t inst)
{
    /* This is a PC-region branch (not PC-relative); the effective target address
       is in the “current” 256 MB-aligned region. The low 28 bits of the target address
       is the instr_index field shifted left 2 bits. The remaining upper bits are the
       corresponding bits of the address of the instruction in the delay slot (not the
       branch itself). Jump to the effective target address. Execute the instruction that
       follows the jump, in the branch delay slot, before executing the jump itself. */
    uint32_t instr_index = decode_instruction_instr_index(inst);
    // the mask f0000000 is for the most significant 4 bits
    cpu->pc = (cpu->pc & 0xf0000000) | instr_index << 2;

    printf("j 0x%x\n", cpu->pc);
}

static void op_or(cpu_t *cpu, instruction_t inst)
{
    uint32_t rs = decode_instruction_rs(inst);
    uint32_t rt = decode_instruction_rt(inst);
    uint32_t rd = decode_instruction_rd(inst);

    cpu->reg[rd] = cpu->reg[rs] | cpu->reg[rt];
    cpu->reg[0] = 0; // $zero is always 0

    printf("or $%d, $%d, $%d\n", rd, rs, rt);
}

static void op_mtc0(cpu_t *cpu, instruction_t inst)
{
    // Move to Coprocessor 0
    uint32_t rt = decode_instruction_rt(inst);
    uint32_t rd = decode_instruction_rd(inst);
    uint32_t sel = decode_instruction_sel(inst);

    if (sel != 0) {
        err_debug("op_mtc0: Not implemented for sel not zero."
                  "sel = %d", sel);
    }

    cpu->cop0.reg[rd] = cpu->reg[rt];
    printf("mtc0 $%d, $cop0_%d\n", rt, rd);
}

static void op_cop0(cpu_t *cpu, instruction_t inst)
{
    // 5 bits [25:21] decode the COP0 operation
    uint32_t cop0_op = decode_instruction_rs(inst);
    global_cop0_optable[cop0_op](cpu, inst);
}

static void op_bne(cpu_t *cpu, instruction_t inst)
{
    uint32_t rs = decode_instruction_rs(inst);
    uint32_t rt = decode_instruction_rt(inst);
    int16_t offset = (int16_t) decode_instruction_imm(inst);

    uint32_t target = offset << 2;

    uint32_t if_branch_pc = cpu->pc;
    if (cpu->reg[rs] != cpu->reg[rt]) {
        if_branch_pc += target;
    }
    cpu->pc = if_branch_pc;

    printf("bne $%d, $%d, 0x%0x\n", rs, rt, if_branch_pc); // print branch PC (pseudo-instruction, my assembly)
    //printf("bne $%d, $%d, %d\n", rs, rt, target); // print the offset from old PC
}

static void op_addi(cpu_t *cpu, instruction_t inst)
{
    // To add a constant to a 32-bit integer. If overflow occurs, then trap.
    // The 16-bit signed immediate is added to the 32-bit value in GPR rs to produce a 32-bit result.
    //   * If the addition results in 32-bit 2’s complement arithmetic overflow, the destination register
    //     is not modified and an Integer Overflow exception occurs.
    //   * If the addition does not overflow, the 32-bit result is placed into GPR rt.
    uint32_t rs = decode_instruction_rs(inst);
    uint32_t rt = decode_instruction_rt(inst);
    int16_t imm = (int16_t) decode_instruction_imm(inst);

    if (detect_overflow_i32((int32_t)cpu->reg[rs], imm)) {
        // Integer Overflow exception, but we will handle that later
        err_debug("op_addi: Integer Overflow Exception caught");
    }
    else {
        cpu->reg[rt] = cpu->reg[rs] + imm;
        cpu->reg[0] = 0; // $zero is always 0
    }

    printf("addi $%d, $%d, 0x%x\n", rt, rs, imm);
}

static void init_optable(op_table_t *optable)
{
    for (int i = 0; i < 64; i++) {
        optable[i] = op_unhandled;
    }
    optable[0]  = op_special; // 0 = (000000)_2 -> SPECIAL (depends on last 6 bits)
    optable[2]  = op_j; // 2 = (000010)_2 -> J (Jump)
    optable[5]  = op_bne; // 5 = (000101)_2 -> BNE (Branch if Not Equal)
    optable[8]  = op_addi; // 8 = (001000)_2 -> ADDI (Add Immediate Word)
    optable[9]  = op_addiu; // 9 = (001001)_2 -> ADDIU (Add Immediate Unsigned Word)
    optable[16] = op_cop0; // 16 = (010000)_2 -> COP0 (Coprocessor 0 Subinstructions)
    optable[13] = op_ori; // 13 = (001101)_2 -> ORI (Or Immediate)
    optable[15] = op_lui; // 15 = (001111)_2 -> LUI (Load Upper Immediate)
    optable[43] = op_sw;  // 43 = (101011)_2 -> SW (Store Word)

    for (int i = 0; i < 64; i++) {
        global_special_optable[i] = op_unhandled;
    }
    global_special_optable[0] = op_sll; // 0 = (000000)_2 = SLL (Shift Word Left Logical)
    global_special_optable[37] = op_or; // 37 = (100101)_2 = OR (Or)

    for (int i = 0; i < 32; i++) {
        global_cop0_optable[i] = op_unhandled;
    }
    global_cop0_optable[4] = op_mtc0; // 4 = (00100)_2 = MTC0 (Move to Coprocessor 0)
}
