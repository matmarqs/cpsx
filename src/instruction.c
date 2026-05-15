#include "instruction.h"

uint32_t decode_instruction_opcode(instruction_t inst)
{
    // bits [31:26]
    return (inst.u32 >> 26);
}

uint32_t decode_instruction_rs(instruction_t inst)
{
    // bits [25:21]
    return (inst.u32 >> 21) & 0x1f;
}

uint32_t decode_instruction_rt(instruction_t inst)
{
    // bits [20:16]
    return (inst.u32 >> 16) & 0x1f;
}

uint32_t decode_instruction_imm(instruction_t inst)
{
    // bits [15:0]
    return inst.u32 & 0xffff;
}
