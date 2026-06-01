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
    cpu_set_reg(cpu, reg_index, (value << 16) & 0xffff0000);

    printf("lui $%d, 0x%x\n", reg_index, value);
}

static void op_ori(cpu_t *cpu, instruction_t inst)
{
    uint32_t rs = decode_instruction_rs(inst);
    uint32_t rt = decode_instruction_rt(inst);
    uint32_t imm = decode_instruction_imm(inst);
    cpu_set_reg(cpu, rt, cpu_reg(cpu, rs) | imm);

    printf("ori $%d, $%d, 0x%x\n", rt, rs, imm);
}

static void op_sw(cpu_t *cpu, instruction_t inst)
{
    // sw rt, offset(rs)
    uint32_t base = decode_instruction_rs(inst);
    uint32_t rt = decode_instruction_rt(inst);
    int16_t offset = (int16_t) decode_instruction_imm(inst);; // 16-bit signed offset

    if ((cpu->cop0.regs[12] & 0x10000) != 0) { // $cop0_12 is the status register
        // Cache is isolated, ignore write
        printf("sw $%d, %hd($%d);; but ignoring store while cache is isolated\n", rt, offset, base);
        return;
    }

    cpu_store32(cpu, cpu_reg(cpu, base) + offset, cpu_reg(cpu, rt));
    printf("sw $%d, %hd($%d)\n", rt, offset, base);
}

static void op_sh(cpu_t *cpu, instruction_t inst)
{
    uint32_t base = decode_instruction_rs(inst);
    uint32_t rt = decode_instruction_rt(inst);
    int16_t offset = (int16_t) decode_instruction_imm(inst);; // 16-bit signed offset

    if ((cpu->cop0.regs[12] & 0x10000) != 0) { // $cop0_12 is the status register
        // Cache is isolated, ignore write
        printf("sh $%d, %hd($%d);; but ignoring store while cache is isolated\n", rt, offset, base);
        return;
    }

    cpu_store16(cpu, cpu_reg(cpu, base) + offset, (uint16_t) cpu_reg(cpu, rt));
    printf("sh $%d, %hd($%d)\n", rt, offset, base);
}

static void op_sb(cpu_t *cpu, instruction_t inst)
{
    uint32_t base = decode_instruction_rs(inst);
    uint32_t rt = decode_instruction_rt(inst);
    int16_t offset = (int16_t) decode_instruction_imm(inst);; // 16-bit signed offset

    if ((cpu->cop0.regs[12] & 0x10000) != 0) { // $cop0_12 is the status register
        // Cache is isolated, ignore write
        printf("sb $%d, %hd($%d);; but ignoring store while cache is isolated\n", rt, offset, base);
        return;
    }

    cpu_store8(cpu, cpu_reg(cpu, base) + offset, (uint8_t) cpu_reg(cpu, rt));
    printf("sb $%d, %hd($%d)\n", rt, offset, base);
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
    cpu_set_reg(cpu, rd, cpu_reg(cpu, rt) << sa);

    if (!rt && !rd && !sa) { // sll $0, $0, 0 is a nop instruction
        printf("nop\n");
    }
    else {
        printf("sll $%d, $%d, $%d\n", rd, rt, sa);
    }
}

static void op_sltu(cpu_t *cpu, instruction_t inst)
{
    uint32_t rs = decode_instruction_rs(inst);
    uint32_t rt = decode_instruction_rt(inst);
    uint32_t rd = decode_instruction_rd(inst);
    cpu_set_reg(cpu, rd, (uint32_t) (cpu_reg(cpu, rs) < cpu_reg(cpu, rt))); // store bool (1 or 0) in rd
    printf("sltu $%d, $%d, $%d\n", rd, rs, rt);
}

static void op_addu(cpu_t *cpu, instruction_t inst)
{
    uint32_t rs = decode_instruction_rs(inst);
    uint32_t rt = decode_instruction_rt(inst);
    uint32_t rd = decode_instruction_rd(inst);
    cpu_set_reg(cpu, rd, cpu_reg(cpu, rs) + cpu_reg(cpu, rt));
    printf("addu $%d, $%d, $%d\n", rd, rs, rt);
}

static void op_addiu(cpu_t *cpu, instruction_t inst)
{
    // The 16-bit signed immediate is added to the 32-bit value in GPR rs
    // and the 32-bit arithmetic result is placed into GPR rt.
    // No Integer Overflow exception occurs under any circumstances.
    uint32_t rs = decode_instruction_rs(inst);
    uint32_t rt = decode_instruction_rt(inst);
    int16_t imm = (int16_t) decode_instruction_imm(inst);
    cpu_set_reg(cpu, rt, cpu_reg(cpu, rs) + imm);

    printf("addiu $%d, $%d, %hd\n", rt, rs, imm);
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

static void op_jal(cpu_t *cpu, instruction_t inst)
{
    uint32_t instr_index = decode_instruction_instr_index(inst);
    cpu_set_reg(cpu, 31, cpu->pc); // register $31 is $ra (return address)
    cpu->pc = (cpu->pc & 0xf0000000) | instr_index << 2;

    printf("jal 0x%x\n", cpu->pc);
}

static void op_jr(cpu_t *cpu, instruction_t inst)
{
    uint32_t rs = decode_instruction_rs(inst);
    cpu->pc = cpu_reg(cpu, rs);

    printf("jr $%d\n", rs);
}

static void op_or(cpu_t *cpu, instruction_t inst)
{
    uint32_t rs = decode_instruction_rs(inst);
    uint32_t rt = decode_instruction_rt(inst);
    uint32_t rd = decode_instruction_rd(inst);

    cpu_set_reg(cpu, rd, cpu_reg(cpu, rs) | cpu_reg(cpu, rt));

    printf("or $%d, $%d, $%d\n", rd, rs, rt);
}

static void op_mtc0(cpu_t *cpu, instruction_t inst)
{
    // Move to Coprocessor 0
    uint32_t rt = decode_instruction_rt(inst);
    uint32_t rd = decode_instruction_rd(inst);

    uint32_t value = cpu_reg(cpu, rt);

    switch (rd) { // rd is the cop0 register
    case 3:
    case 5:
    case 6:
    case 7:
    case 9:
    case 11:
    case 13:
        if (value != 0) {
            err_debug("op_mtc0: Unhandled write to $cop0_%d", rd);
        }
        break;
    case 12:
        cpu->cop0.regs[rd] = value;
        break;
    default:
        err_debug("Unhandled $cop0_%d register");
        break;
    }

    printf("mtc0 $%d, $cop0_%d\n", rt, rd);
}

static void op_cop0(cpu_t *cpu, instruction_t inst)
{
    // 5 bits [25:21] decode the COP0 operation
    uint32_t cop0_op = decode_instruction_rs(inst);
    global_cop0_optable[cop0_op](cpu, inst);
}

static void op_beq(cpu_t *cpu, instruction_t inst)
{
    uint32_t rs = decode_instruction_rs(inst);
    uint32_t rt = decode_instruction_rt(inst);
    int16_t offset = (int16_t) decode_instruction_imm(inst);

    uint32_t target = offset << 2;
    uint32_t if_branch_pc = (cpu->pc - 4) + target; // SUBTRACT 4, COMPENSATE FOR += 4 at main loop

    if (cpu_reg(cpu, rs) == cpu_reg(cpu, rt)) {
        cpu->pc = if_branch_pc;
    }

    printf("beq $%d, $%d, 0x%0x\n", rs, rt, if_branch_pc); // print absolute address (pseudo-instruction)

}

static void op_bne(cpu_t *cpu, instruction_t inst)
{
    uint32_t rs = decode_instruction_rs(inst);
    uint32_t rt = decode_instruction_rt(inst);
    int16_t offset = (int16_t) decode_instruction_imm(inst);

    uint32_t target = offset << 2;
    uint32_t if_branch_pc = (cpu->pc - 4) + target; // SUBTRACT 4, COMPENSATE FOR += 4 at main loop

    if (cpu_reg(cpu, rs) != cpu_reg(cpu, rt)) {
        cpu->pc = if_branch_pc;
    }

    printf("bne $%d, $%d, 0x%0x\n", rs, rt, if_branch_pc); // print absolute address (pseudo-instruction)
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

    if (detect_overflow_i32((int32_t)cpu_reg(cpu, rs), imm)) {
        // Integer Overflow exception, but we will handle that later
        err_debug("op_addi: Integer Overflow Exception caught");
    }
    else {
        cpu_set_reg(cpu, rt, cpu_reg(cpu, rs) + imm);
    }

    printf("addi $%d, $%d, %hd\n", rt, rs, imm);
}

static void op_andi(cpu_t *cpu, instruction_t inst)
{
    uint32_t rs = decode_instruction_rs(inst);
    uint32_t rt = decode_instruction_rt(inst);
    uint32_t imm = decode_instruction_imm(inst);
    cpu_set_reg(cpu, rt, cpu_reg(cpu, rs) & imm);
    printf("andi $%d, $%d, 0x%x\n", rt, rs, imm);
}

static void op_lw(cpu_t *cpu, instruction_t inst)
{
    // https://www.reddit.com/r/EmuDev/comments/1fxhncn/how_does_the_ps1_load_delay_slots_work/
    // Load Word instruction has delay slots

    // From: https://github.com/simias/psx-guide, Section 2.31 Memory Loads
    /* Instead I’m going to use two sets of general purpose registers: one will be
       the input set and the other the output set. Each instruction will read its input
       values from the former set and will write to the latter. Once the instruction is
       finished we copy the output set into the input set for the next instruction.
       This way we can update the output register set with the load value before
       we execute the instruction and it will still see the old value from the input set.
       And if the instruction writes to the same register it will overwrite the value in
       the output set. */

    uint32_t base = decode_instruction_rs(inst);
    uint32_t rt = decode_instruction_rt(inst);
    int16_t offset = (int16_t) decode_instruction_imm(inst);; // 16-bit signed offset

    if ((cpu->cop0.regs[12] & 0x10000) != 0) { // $cop0_12 is the status register
        // Cache is isolated, ignore write
        printf("lw $%d, %hd($%d) ;; but ignoring load while cache is isolated\n",
               rt, offset, base);
        return;
    }

    uint32_t addr = cpu_reg(cpu, base) + offset;
    uint32_t value = cpu_load32(cpu, addr);

    // Schedule the load for next instruction
    cpu_load_delay(cpu, rt, value);

    printf("lw $%d, %hd($%d)\n", rt, offset, base);
}

static void op_lb(cpu_t *cpu, instruction_t inst)
{
    uint32_t base = decode_instruction_rs(inst);
    uint32_t rt = decode_instruction_rt(inst);
    int16_t offset = (int16_t) decode_instruction_imm(inst);; // 16-bit signed offset

    if ((cpu->cop0.regs[12] & 0x10000) != 0) { // $cop0_12 is the status register
        // Cache is isolated, ignore write
        printf("lb $%d, %hd($%d);; but ignoring load while cache is isolated\n",
               rt, offset, base);
        return;
    }

    uint32_t addr = cpu_reg(cpu, base) + offset;
    int8_t value = (int8_t) cpu_load8(cpu, addr); // force sign extension

    cpu_load_delay(cpu, rt, (uint32_t) value);

    printf("lb $%d, %hd($%d)\n", rt, offset, base);
}

static void init_optable(op_table_t *optable)
{
    for (int i = 0; i < 64; i++) {
        optable[i] = op_unhandled;
    }
    optable[0]  = op_special; // 0 = (000000)_2 -> SPECIAL (depends on last 6 bits)
    optable[2]  = op_j; // 2 = (000010)_2 -> J (Jump)
    optable[3]  = op_jal; // 3 = (000011)_2 -> JAL (Jump and Link)
    optable[4]  = op_beq; // 4 = (000100)_2 -> BEQ (Branch on Equal)
    optable[5]  = op_bne; // 5 = (000101)_2 -> BNE (Branch if Not Equal)
    optable[8]  = op_addi; // 8 = (001000)_2 -> ADDI (Add Immediate Word)
    optable[9]  = op_addiu; // 9 = (001001)_2 -> ADDIU (Add Immediate Unsigned Word)
    optable[12] = op_andi; // 12 = (001100)_2 -> ANDI (And Immediate)
    optable[16] = op_cop0; // 16 = (010000)_2 -> COP0 (Coprocessor 0 Subinstructions)
    optable[13] = op_ori; // 13 = (001101)_2 -> ORI (Or Immediate)
    optable[15] = op_lui; // 15 = (001111)_2 -> LUI (Load Upper Immediate)
    optable[32] = op_lb; // 32 = (100000)_2 -> LB (Load Byte)
    optable[35] = op_lw;  // 35 = (100011)_2 -> LW (Load Word)
    optable[40] = op_sb;  // 40 = (101000)_2 -> SB (Store Byte)
    optable[41] = op_sh;  // 41 = (101001)_2 -> SH (Store Halfword)
    optable[43] = op_sw;  // 43 = (101011)_2 -> SW (Store Word)

    for (int i = 0; i < 64; i++) {
        global_special_optable[i] = op_unhandled;
    }
    global_special_optable[0] = op_sll; // 0 = (000000)_2 = SLL (Shift Word Left Logical)
    global_special_optable[8] = op_jr; // 8 = (001000)_2 = JR (Jump Register)
    global_special_optable[33] = op_addu; // 33 = (100001)_2 = ADDU (Add Unsigned Word)
    global_special_optable[37] = op_or; // 37 = (100101)_2 = OR (Or)
    global_special_optable[43] = op_sltu; // 43 = (101011)_2 = SLTU (Set on Less Than Unsigned)

    for (int i = 0; i < 32; i++) {
        global_cop0_optable[i] = op_unhandled;
    }
    global_cop0_optable[4] = op_mtc0; // 4 = (00100)_2 = MTC0 (Move to Coprocessor 0)
}
