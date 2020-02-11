#ifndef __CS261_P2__
#define __CS261_P2__

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "elf.h"
#include "y86.h"

bool read_phdr (FILE *file, uint16_t offset, elf_phdr_t *phdr);
bool load_segment (FILE *file, byte_t *memory, elf_phdr_t phdr);

void usage_p2 (char **argv);
bool parse_command_line_p2 (int argc, char **argv,
        bool *header, bool *segments, bool *membrief, bool *memfull,
        char **filename);
void dump_phdrs (uint16_t numphdrs, elf_phdr_t phdr[]);
void dump_memory (byte_t *memory, uint16_t start, uint16_t end);

#endif
