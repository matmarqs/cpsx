#ifndef _OP_H
#define _OP_H

#include "types.h"

extern op_table_t global_optable[64];
extern op_table_t global_special_optable[64];
void op_init(void);

#endif
