#ifndef __CS261_P1__
#define __CS261_P1__

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "elf.h"

bool read_header (FILE *file, elf_hdr_t *hdr);

void usage_p1 (char **argv);
bool parse_command_line_p1 (int argc, char **argv, bool *header, char **filename);
void dump_header (elf_hdr_t hdr);

#endif
