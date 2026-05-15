#include "error.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

uint8_t bios[512 * 1024];

bool read_bios(const char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp) return false;

    fseek(fp, 0L, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    fread(bios, 1, size, fp);

    fclose(fp);

    return true;
}

uint32_t load32(uint32_t offset)
{
    if ((offset & 0xf) != 0) {
        err_quit("Unaligned memory access: 0x%08x", offset);
    }
    uint8_t a = bios[offset + 0];
    uint8_t b = bios[offset + 1];
    uint8_t c = bios[offset + 2];
    uint8_t d = bios[offset + 3];

    // 0x12345678 in memory -> 0x78563412

    return (d << 24) | (c << 16) | (b << 8) | a;
}

int main(void)
{
    if (!read_bios("SCPH1001.BIN")) {
        return 1;
    }

    printf("First BIOS instruction: 0x%08x\n", load32(0));

    return 0;
}
