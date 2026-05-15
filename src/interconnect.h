#ifndef _INTERCONNECT_H
#define _INTERCONNECT_H

#include "types.h"

uint32_t interconnect_load32(inter_t *inter, uint32_t offset);
uint32_t interconnect_store32(inter_t *inter, uint32_t offset, uint32_t value);
void interconnect_init(inter_t *inter);

#endif // _INTERCONNECT_H
