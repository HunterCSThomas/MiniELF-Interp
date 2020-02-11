#ifndef __CS261_P3__
#define __CS261_P3__

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "elf.h"
#include "y86.h"

y86_inst_t fetch (y86_t *cpu, byte_t *memory);

void usage_p3 (char **argv);
bool parse_command_line_p3 (int argc, char **argv,
        bool *header, bool *segments, bool *membrief, bool *memfull,
        bool *disas_code, bool *disas_data, char **filename);
void disassemble (y86_inst_t inst);
void disassemble_code   (byte_t *memory, elf_phdr_t *phdr, elf_hdr_t *hdr);
void disassemble_data   (byte_t *memory, elf_phdr_t *phdr);
void disassemble_rodata (byte_t *memory, elf_phdr_t *phdr);

#endif
