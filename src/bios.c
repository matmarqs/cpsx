#include "bios.h"
#include "util.h"

uint8_t global_bios[BIOS_SIZE];

bool read_bios(const char *filename)
{
    return read_file(filename, global_bios, BIOS_SIZE);
}
