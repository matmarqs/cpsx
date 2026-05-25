#include "bios.h"
#include "util.h"

bool read_bios(const char *filename)
{
    return read_file(filename, global_bios, PSX_SIZE_BIOS);
}
