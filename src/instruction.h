#ifndef _INSTRUCTION_H
#define _INSTRUCTION_H

#include "types.h"

uint32_t decode_instruction_opcode(instruction_t inst);
uint32_t decode_instruction_rs(instruction_t inst);
uint32_t decode_instruction_rt(instruction_t inst);
uint32_t decode_instruction_imm(instruction_t inst);

#endif // _INSTRUCTION_H
