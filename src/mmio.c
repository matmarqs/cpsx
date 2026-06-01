#include "mmio.h"

#include <string.h>

uint8_t global_memcontrol[PSX_SIZE_MEMCONTROL];
uint8_t global_expregion1[PSX_SIZE_EXPREGION1];
uint8_t global_expregion2[PSX_SIZE_EXPREGION2];
uint8_t global_regcontrol[PSX_SIZE_REGCONTROL];
uint8_t global_mainram[PSX_SIZE_MAINRAM];
uint8_t global_bios[PSX_SIZE_BIOS];

void expregion1_init(void)
{
    // Address  Size Content
    // 1F000000h 4   Post-Boot Entrypoint (eg. 1F000100h and up)
    // 1F000004h 2Ch Post-Boot ID ("Licensed by Sony Computer Entertainment Inc.")
    // 1F000030h 50h Post-Boot TTY Message (must contain at least one 00h byte)
    // 1F000080h 4   Pre-Boot Entrypoint  (eg. 1F000100h and up)
    // 1F000084h 2Ch Pre-Boot ID  ("Licensed by Sony Computer Entertainment Inc.")
    // 1F0000B0h 50h Not used     (should be zero, but may contain code/data/io)
    // 1F000100h ..  Code, Data, I/O Ports, etc.
    const char *header_string = "Licensed by Sony Computer Entertainment Inc.";
    memcpy(&global_expregion1[0x04], header_string, 44);
    memcpy(&global_expregion1[0x84], header_string, 44);
    *(uint32_t *)(global_expregion1 + 0x00) = 0x1f000100; // post-boot entrypoint
    *(uint32_t *)(global_expregion1 + 0x80) = 0x1f000100; // pre-boot entrypoint
}
