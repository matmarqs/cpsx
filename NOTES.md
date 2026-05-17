# notes about the PSX

## references

* <https://psx-spx.consoledev.net/>
* <https://gamingdoc.org/technical-documentation/consoles/sony-playstation/>.
* <https://psx.schnappy.xyz/>.
* A good resource on the BIOS is [pcsx-redux](https://github.com/grumpycoders/pcsx-redux/blob/main/src/mips/openbios/boot/psx.s).

## bios

### info

* Main reference in <https://psx-spx.consoledev.net/memorycontrol/>.

The first instructions of the BIOS are

```
0xbfc00000: 0x3c080013    lui $8, 0x13
0xbfc00004: 0x3508243f    ori $8, $8, 0x243f
0xbfc00008: 0x3c011f80    lui $1, 0x1f80
0xbfc0000c: 0xac281010    sw  $8, 0x1010($1)
```

The memory region `0x1f801000 -- 0x1f802fff` is for hardware registers (or I/O ports).

On <https://psx-spx.consoledev.net/memorycontrol/>, we have this text:

1F801010h - BIOS ROM Delay/Size (usually 0013243Fh) (512Kbytes, 8bit bus)

```
  0-3   Write Delay        (00h..0Fh=01h..10h Cycles)
  4-7   Read Delay         (00h..0Fh=01h..10h Cycles)
  8     Recovery Period    (0=No, 1=Yes, uses COM0 timings)
  9     Hold Period        (0=No, 1=Yes, uses COM1 timings)
  10    Floating Period    (0=No, 1=Yes, uses COM2 timings)
  11    Pre-strobe Period  (0=No, 1=Yes, uses COM3 timings)
  12    Data Bus-width     (0=8bits, 1=16bits)
  13    Auto Increment     (0=No, 1=Yes)
  14-15 Unknown (R/W)
  16-20 Number of address bits (memory window size = 1 << N bytes)
  21-23 Unknown (always zero)
  24-27 DMA timing override
  28    Address error flag. Write 1 to it to clear it.
  29    DMA timing select  (0=use normal timings, 1=use bits 24-27)
  30    Wide DMA           (0=use bit 12, 1=override to full 32 bits)
  31    Wait               (1=wait on external device before being ready)
```

The BIOS stores `0x0013243f` at memory address `0x1f801010`. `0x13` means the BIOS size `2^19 = 512 KB`. `0x243f` seems to be the BIOS bus flags configuration.

```
0x0013 = 19 --> 2^19 = 512 KB
0x243f = 0010 0100 0011 1111

0-3   Write Delay             1111 = 0xf = 16 cycles
                              
4-7   Read Delay              0011 = 0x3 = 4 cycles
                              
8     Recovery Period         0 = No
9     Hold Period             0 = No
10    Floating Period         1 = Yes
11    Pre-strobe Period       0 = No
                              
12    Data Bus-width          0 = 8 bits
13    Auto Increment          1 = Yes
14-15 Unknown (R/W)           00
```

So, `0x243f` actually means it's BIOS ROM (512 KB, 8-bit bus).



