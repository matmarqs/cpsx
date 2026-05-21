#include "instruction.h"

uint32_t decode_instruction_opcode(instruction_t inst)
{
    // 6 bits [31:26]
    return (inst.u32 >> 26);
}

uint32_t decode_instruction_rs(instruction_t inst)
{
    // 5 bits [25:21]
    return (inst.u32 >> 21) & MASK_05BITS;
}

uint32_t decode_instruction_rt(instruction_t inst)
{
    // 5 bits [20:16]
    return (inst.u32 >> 16) & MASK_05BITS;
}

uint32_t decode_instruction_imm(instruction_t inst)
{
    // 16 bits [15:0]
    return inst.u32 & MASK_16BITS;
}

uint32_t decode_instruction_special_op(instruction_t inst)
{
    // 6 bits [5:0]
    return inst.u32 & MASK_06BITS;
}

uint32_t decode_instruction_rd(instruction_t inst)
{
    // 5 bits [15:11]
    return (inst.u32 >> 11) & MASK_05BITS;
}

uint32_t decode_instruction_sa(instruction_t inst)
{
    // 5 bits [10:6]
    return (inst.u32 >> 6) & MASK_05BITS;
}

uint32_t decode_instruction_instr_index(instruction_t inst)
{
    // 26 bits [25:0]
    return inst.u32 & MASK_26BITS;
}
