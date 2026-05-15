# cpsx

Writing a PSX emulator.

## roadmap

### what needs to be done

The PSX architecture is very simple. The MIPS R3000A CPU is the brain, so we basically have to implement all its
important instructions first. Some of it instructions will be for the coprocessors, which are:

* CP0 (System Control Coprocessor, required by MIPS): for debugging, handling exceptions and interrupts.
* GTE (Geometry Transformation Engine): this was a custom chip developed by Sony to handle graphical (matrix, vector, etc.) stuff.
* MDEC (Motion Decoder): a chip to handle multimedia stuff.

Aside from these, we will then need to handle memory mapped I/O, which will make use of the GPU, BIOS or other stuff.

The entry point for the PSX is the 512 KB BIOS, which for now I will assume is:

| BIOS file | md5 |
| --------- | --- |
| SCPH1001.BIN | 924e392ed05558ffdb115408c263dccf | 

I read a little bit of the guide <https://github.com/simias/psx-guide>, and the idea is basically to debug the BIOS code
until we have all the things appearing in the screen (we also need to develop a screen, probably with SDL).

## reminder to myself

I will try to not read the guide too much. It is incomplete sure, but I want to discover things on my own.

## references

### main references

* PlayStation Architecture Article: <https://www.copetti.org/writings/consoles/playstation/>.
* Guide for writing a PSX emulator (in Rust): <https://github.com/simias/psx-guide>.
* No$ PSX specifications: <https://problemkaputt.de/psx-spx.htm>.
* PSX technical documentation: <https://gamingdoc.org/technical-documentation/consoles/sony-playstation/>.

### others

* PlayStation Wikipedia Article: <https://en.wikipedia.org/wiki/PlayStation_(console)>.
