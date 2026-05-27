#ifndef _CPU_H
#define _CPU_H

#include "types.h"

void cpu_print_state(cpu_t *cpu);
uint32_t cpu_load32(cpu_t *cpu, uint32_t mem_location);
bool cpu_store32(cpu_t *cpu, uint32_t offset, uint32_t value);
uint32_t cpu_reg(cpu_t *cpu, uint8_t index);
void cpu_set_reg(cpu_t *cpu, uint8_t index, uint32_t value);
void cpu_load_delay(cpu_t *cpu, uint8_t reg_index, uint32_t value);
void cpu_init(cpu_t *cpu, inter_t *interconnect);
void cpu_main(cpu_t *cpu);

#endif // _CPU_H
