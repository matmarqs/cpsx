#include "bios.h"
#include "util.h"

uint8_t global_bios[PSX_SIZE_BIOS];

bool read_bios(const char *filename)
{
    return read_file(filename, global_bios, PSX_SIZE_BIOS);
}
