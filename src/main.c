#include "cpu.h"

int main(void)
{
    inter_t interconnect;
    cpu_t cpu;

    cpu_init(&cpu, &interconnect);

    cpu_main(&cpu);

    return 0;
}
