#ifndef _INTERCONNECT_H
#define _INTERCONNECT_H

#include "types.h"

uint32_t interconnect_load32(inter_t *inter, uint32_t mem_location);
bool interconnect_store32(inter_t *inter, uint32_t offset, uint32_t value);
bool interconnect_store16(inter_t *inter, uint32_t addr, uint16_t value);
bool interconnect_store8(inter_t *inter, uint32_t addr, uint8_t value);
uint8_t interconnect_load8(inter_t *inter, uint32_t addr);
void interconnect_init(inter_t *inter);

#endif // _INTERCONNECT_H
