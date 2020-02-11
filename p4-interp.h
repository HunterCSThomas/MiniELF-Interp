#ifndef __CS261_P4__
#define __CS261_P4__

#include <getopt.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "elf.h"
#include "y86.h"

y86_reg_t decode_execute (y86_t *cpu, bool *cond, y86_inst_t inst,
        y86_reg_t *valA);
void memory_wb_pc (y86_t *cpu, byte_t *memory, bool cond, y86_inst_t inst,
        y86_reg_t valE, y86_reg_t valA);

void usage_p4 (char **argv);
bool parse_command_line_p4 (int argc, char **argv,
        bool *header, bool *segments, bool *membrief, bool *memfull,
        bool *disas_code, bool *disas_data,
        bool *exec_normal, bool *exec_debug, char **filename);
void dump_memory_write(address_t addr, y86_reg_t value);
void dump_cpu_state (y86_t cpu);

#endif
