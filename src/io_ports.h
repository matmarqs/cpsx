#ifndef _IO_PORTS_H
#define _IO_PORTS_H

#include <stdint.h>

// KUSEG     KSEG0     KSEG1
// 1F801000h 9F801000h BF801000h  8K     I/O Ports

// region: 0x1f801000 -- 0x1f803000

// I/O Ports
#define IO_PORTS1_ADDR 0x1F801000
#define IO_PORTS1_SIZE (4*1024)

extern uint8_t global_io_ports_1[IO_PORTS1_SIZE];

// Expansion Region 2
#define IO_PORTS2_ADDR 0x1F802000
#define IO_PORTS2_SIZE (8*1024)

extern uint8_t global_io_ports_2[IO_PORTS2_SIZE];

// Expansion Region 3 (SRAM BIOS region for DTL cards)
#define IO_PORTS3_ADDR 0x1FA00000
#define IO_PORTS3_SIZE (2048*1024)

#endif // _IO_PORTS_H
