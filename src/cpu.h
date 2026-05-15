#ifndef _CPU_H
#define _CPU_H

#include "types.h"

void cpu_print_state(cpu_t *cpu);
uint32_t cpu_load32(cpu_t *cpu, uint32_t offset);
uint32_t cpu_store32(cpu_t *cpu, uint32_t offset, uint32_t value);
void cpu_init(cpu_t *cpu, inter_t *interconnect);
void cpu_main(cpu_t *cpu);

#endif // _CPU_H
