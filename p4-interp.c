/*
 * CS 261 PA4: Mini-ELF interpreter
 *
 * Name: Hunter Thomas
 */

#include "p4-interp.h"
bool isvalidaddress(y86_reg_t address);
bool isvalid(y86_inst_t *ins, y86_t *cpu);
void writemem(byte_t *memory, y86_reg_t address, y86_reg_t *value, bool to);

/**********************************************************************
 *                         REQUIRED FUNCTIONS
 *********************************************************************/

y86_reg_t decode_execute (y86_t *cpu, bool *cond, y86_inst_t inst,
                          y86_reg_t *valA)
{
    y86_reg_t valE = 0;
    y86_reg_t valB = 0;
    bool halt = false;

    if (cond == NULL || valA == NULL) {
        cpu->stat = INS;
        return -1;
    }

    if (!isvalid(&inst, cpu)) {
        return -1;
    }

    switch(inst.icode) {
        case HALT:
            halt = true;
            break;
        case NOP:
            break;
        case CMOV:
            *valA = cpu->reg[inst.ra];
            switch(inst.cmov) {
                case RRMOVQ:
                    *cond = true;
                    break;
                case CMOVLE:
                    *cond = (cpu->sf ^ cpu->of) | cpu->zf;
                    break;
                case CMOVL:
                    *cond = cpu->sf ^ cpu->of;
                    break;
                case CMOVE:
                    *cond = cpu->zf;
                    break;
                case CMOVNE:
                    *cond = !(cpu->zf);
                    break;
                case CMOVGE:
                    *cond = !(cpu->sf ^ cpu->of);
                    break;
                case CMOVG:
                    *cond = !(cpu->sf ^ cpu->of) & !(cpu->zf);
                    break;
                case BADCMOV:
                    break;
            }
            valE = *valA;
            break;
        case IRMOVQ:
            valE = (y86_reg_t)inst.v;
            break;
        case RMMOVQ:
            *valA = cpu->reg[inst.ra];
            valB = cpu->reg[inst.rb];
            valE = valB + inst.d;
            break;
        case MRMOVQ:
            valB = cpu->reg[inst.rb];
            valE = valB + inst.d;
            break;
        case OPQ:
            *valA = cpu->reg[inst.ra];
            valB =  cpu->reg[inst.rb];
            //switch opcode preform op and set CC
            switch(inst.op) {
                case ADD:
                    valE = *valA + valB;
                    if ((valB > 0) && (*valA > 0) && (valE >> 63 == 1)) {
                        cpu->of = true;
                    } else if ((valB < 0) && (*valA < 0) && (valE >> 63 == 0)) {
                        cpu->of = true;
                    }
                    break;
                case SUB:
                    valE = valB - *valA;
                    if ((valB < 0) && (*valA > 0) && (valE >> 63 == 1)) {
                        cpu->of = true;
                    } else if ((valB > 0) && (*valA < 0) && (valE >> 63 == 0)) {
                        cpu->of = true;
                    }
                    break;
                case AND:
                    valE = *valA & valB;
                    break;
                case XOR:
                    valE = *valA ^ valB;
                    break;
                case BADOP:
                    cpu->stat = INS;
                    return 0;
                    break;
            }
            cpu->sf = valE>>63;
            cpu->zf = (valE==0);
            break;
        case JUMP:
            switch(inst.jump) {
                case JMP:
                    *cond = true;
                    break;
                case JLE:
                    *cond = (cpu->sf ^ cpu->of) | cpu->zf;
                    break;
                case JL:
                    *cond = cpu->sf ^ cpu->of;
                    break;
                case JE:
                    *cond = cpu->zf;
                    break;
                case JNE:
                    *cond = !(cpu->zf);
                    break;
                case JGE:
                    *cond = !(cpu->sf ^ cpu->of);
                    break;
                case JG:
                    *cond = !(cpu->sf ^ cpu->of) & !(cpu->zf);
                    break;
                case BADJUMP:
                    cpu->stat = INS;
                    return 0;
                    break;
            }
            break;
        case CALL:
            valB = cpu->reg[RSP];
            valE = valB - 8;
            break;
        case RET:
            *valA = cpu->reg[RSP];
            valB = cpu->reg[RSP];
            valE = valB + 8;
            break;
        case PUSHQ:
            *valA = cpu->reg[inst.ra];
            valB = cpu->reg[RSP];
            valE = valB - 8;
            break;
        case POPQ:
            *valA = cpu->reg[RSP];
            valB = cpu->reg[RSP];
            valE = valB + 8;
            break;
        case IOTRAP:
            break;
        case INVALID:
            return 0;
            break;
    }

    if (halt) {
        cpu->stat = HLT;
        cpu->sf = 0;
        cpu->zf = 0;
        cpu->of = 0;
    } else {
        cpu->stat = AOK;
    }
    return valE;
}

//perform the memory write back and program counter steps of the cycle
void memory_wb_pc (y86_t *cpu, byte_t *memory, bool cond, y86_inst_t inst,
                   y86_reg_t valE, y86_reg_t valA)
{
    y86_reg_t valM = 0;
    y86_reg_t valP = cpu->pc + inst.size;

    //validate memory and program counter
    if (memory == NULL || !isvalidaddress(cpu->pc)) {
        cpu->pc = 0;
        cpu->stat = ADR;
        return;
    }

    switch(inst.icode) {
        case HALT:
            cpu->pc = 0;
            break;
        case NOP:
            cpu->pc = valP;
            break;
        case CMOV:
            if (cond) {
                cpu->reg[inst.rb] = valE;
            }
            cpu->pc = valP;
            break;
        case IRMOVQ:
            cpu->reg[inst.rb] = valE;
            cpu->pc = valP;
            break;
        case RMMOVQ:
            if (isvalidaddress(valE)) {
                writemem(memory, valE, &valA, true);
            } else {
                cpu->stat = ADR;
                return;
            }
            cpu->pc += inst.size;
            break;
        case MRMOVQ:
            if (isvalidaddress(valE)) {
                writemem(memory, valE, &valM, false);
                cpu->reg[inst.ra] = valM;
                cpu->pc += inst.size;
            } else {
                cpu->stat = ADR;
                cpu->pc += inst.size;
                return;
            }
            break;
        case OPQ:
            cpu->reg[inst.rb] = valE;
            cpu->pc += inst.size;
            break;
        case JUMP:
            if (cond) {
                cpu->pc = inst.dest;
            } else {
                cpu->pc += inst.size;
            }
            break;
        case CALL:
            if (isvalidaddress(valE)) {
                uint64_t temp = cpu->pc + inst.size;
                writemem(memory, valE, &temp, true);
                cpu->reg[RSP] = valE;
            } else {
                cpu->stat = ADR;
                return;
            }
            cpu->pc = inst.dest;
            break;
        case RET:
            if (isvalidaddress(valA)) {
                writemem(memory, valA, &valM, false);
                cpu->reg[RSP] = valE;
            } else {
                cpu->stat = ADR;
                return;
            }
            cpu->pc = valM;
            break;
        case PUSHQ:
            if (isvalidaddress(valE)) {
                writemem(memory, valE, &valA, true);
                cpu->reg[RSP] = valE;
            } else {
                cpu->stat = ADR;
                return;
            }
            cpu->pc += inst.size;
            break;
        case POPQ:
            if (isvalidaddress(valA)) {
                writemem(memory, valA, &valM, false);
                cpu->reg[RSP] = valE;
                cpu->reg[inst.ra] = valM;
            } else {
                cpu->stat = ADR;
                return;
            }
            cpu->pc += inst.size;
        case IOTRAP:
            break;

        case INVALID:
            break;
    }
}

/**********************************************************************
 *                         OPTIONAL FUNCTIONS
 *********************************************************************/

void usage_p4 (char **argv)
{
    printf("Usage: %s <option(s)> mini-elf-file\n", argv[0]);
    printf(" Options are:\n");
    printf("  -h      Display usage\n");
    printf("  -H      Show the Mini-ELF header\n");
    printf("  -a      Show all with brief memory\n");
    printf("  -f      Show all with full memory\n");
    printf("  -s      Show the program headers\n");
    printf("  -m      Show the memory contents (brief)\n");
    printf("  -M      Show the memory contents (full)\n");
    printf("  -d      Disassemble code contents\n");
    printf("  -D      Disassemble data contents\n");
    printf("  -e      Execute program\n");
    printf("  -E      Execute program (trace mode)\n");
}

bool parse_command_line_p4 (int argc, char **argv,
                            bool *header, bool *segments, bool *membrief, bool *memfull,
                            bool *disas_code, bool *disas_data,
                            bool *exec_normal, bool *exec_trace, char **filename)
{
    int opt;
    bool littlem, bigm, littlee, bige = false;

    while ((opt = getopt(argc, argv, "+hHafsmMdDeE")) != -1) {
        switch(opt) {
            case 'h':
                *header = false;
                *filename = NULL;
                return true;
                break;
            case 'H':
                *header = true;
                break;
            case 'a':
                *header = true;
                *segments = true;
                *membrief = true;
                littlem = true;
                break;
            case 'f':
                *header = true;
                *segments = true;
                *memfull = true;
                bigm = true;
                break;
            case 'm':
                *membrief = true;
                littlem = true;
                break;
            case 'M':
                *memfull = true;
                bigm = true;
                break;
            case 'd':
                *disas_code = true;
                break;
            case 'D':
                *disas_data = true;
                break;
            case 'e':
                *exec_normal = true;
                littlee = true;
                break;
            case 'E':
                *exec_trace = true;
                bige = true;
                break;
            default:
                return false;
        }
    }

    if ((littlem && bigm) || (littlee && bige)) {
        return false;
    }

    if (optind != argc - 1) {
        return false;
    }

    *filename = argv[optind];
    return true;
}

//print the contents of cpu to stdout
void dump_cpu_state (y86_t cpu)
{
    printf("Y86 CPU state:\n");
    printf("  %%rip: %016lx   flags: Z%d S%d O%d     ", cpu.pc, cpu.zf, cpu.sf, cpu.of);

    //print the status
    switch(cpu.stat) {
        case AOK:
            printf("AOK\n");
            break;
        case HLT:
            printf("HLT\n");
            break;
        case ADR:
            printf("ADR\n");
            break;
        case INS:
            printf("INS\n");
            break;
    }

    printf("  %%rax: %016lx    %%rcx: %016lx\n", cpu.reg[0], cpu.reg[1]);
    printf("  %%rdx: %016lx    %%rbx: %016lx\n", cpu.reg[2], cpu.reg[3]);
    printf("  %%rsp: %016lx    %%rbp: %016lx\n", cpu.reg[4], cpu.reg[5]);
    printf("  %%rsi: %016lx    %%rdi: %016lx\n", cpu.reg[6], cpu.reg[7]);
    printf("   %%r8: %016lx     %%r9: %016lx\n", cpu.reg[8], cpu.reg[9]);
    printf("  %%r10: %016lx    %%r11: %016lx\n", cpu.reg[10], cpu.reg[11]);
    printf("  %%r12: %016lx    %%r13: %016lx\n", cpu.reg[12], cpu.reg[13]);
    printf("  %%r14: %016lx\n", cpu.reg[14]);

}

//helper method for validating instructions,updates cpu status
bool isvalid(y86_inst_t *ins, y86_t *cpu)
{
    //returns true if valid instruction false otherwise

    if (ins->icode >= INVALID) {
        cpu->stat = INS;
        return false;
    }

    switch(ins->icode) {
        case HALT:
            if ((ins->opcode & 0xF) != 0) {
                cpu->stat = INS;
                return false;
            }
            break;
        case NOP:
            if ((ins->opcode & 0xF) != 0) {
                cpu->stat = INS;
                return false;
            }
            break;
        case CMOV:
            if (ins->cmov >= BADCMOV) {
                cpu->stat = INS;
                return false;
            } else if (!isvalidaddress(cpu->pc + 1)) {
                cpu->stat = ADR;
                return false;
            }
            break;
        case IRMOVQ:
            if ((ins->opcode & 0xF) != 0) {
                cpu->stat = INS;
                return false;
            } else if (!isvalidaddress(cpu->pc + 9)) {
                cpu->stat = ADR;
                return false;
            }
            break;
        case RMMOVQ:
            if ((ins->opcode & 0xF) != 0) {
                cpu->stat = INS;
                return false;
            } else if (!isvalidaddress(cpu->pc + 9)) {
                cpu->stat = ADR;
                return false;
            }
            break;
        case MRMOVQ:
            if ((ins->opcode & 0xF) != 0) {
                cpu->stat = INS;
                return false;
            } else if (!isvalidaddress(cpu->pc + 9)) {
                cpu->stat = ADR;
                return false;
            }
            break;
        case OPQ:
            if (ins->op >= BADOP) {
                cpu->stat = INS;
                return false;
            } else if (!isvalidaddress(cpu->pc + 1)) {
                cpu->stat = ADR;
                return false;
            }
            break;
        case JUMP:
            if (ins->jump >= BADJUMP) {
                cpu->stat = INS;
                return false;
            } else if (!isvalidaddress(cpu->pc + 8)) {
                cpu->stat = ADR;
                return false;
            }
            break;
        case CALL:
            if ((ins->opcode & 0xF) != 0) {
                cpu->stat = INS;
                return false;
            } else if (!isvalidaddress(cpu->pc + 8)) {
                cpu->stat = ADR;
                return false;
            }
            break;
        case RET:
            if ((ins->opcode & 0xF) != 0) {
                cpu->stat = INS;
                return false;
            }
            break;
        case PUSHQ:
            if ((ins->opcode & 0xF) != 0) {
                cpu->stat = INS;
                return false;
            } else if (!isvalidaddress(cpu->pc + 1)) {
                cpu->stat = ADR;
                return false;
            }
            break;
        case POPQ:
            if ((ins->opcode & 0xF) != 0) {
                cpu->stat = INS;
                return false;
            } else if (!isvalidaddress(cpu->pc + 1)) {
                cpu->stat = ADR;
                return false;
            }
            break;
        case IOTRAP:
            if (ins->id >= BADTRAP) {
                cpu->stat = INS;
                return false;
            }
            break;
        case INVALID:
            cpu->stat = INS;
            return false;
            break;
    }

    return true;

}

//helper function for writing an 8 byte value to/from memory
void writemem(byte_t *memory, y86_reg_t address, y86_reg_t *value, bool to)
{
    if (to) {
        for (uint8_t i = 0; i < 8; i++) {
            memory[address + i] = *value >> i*8 & 0xFF;
        }
    } else {
        *value = 0;
        for (uint8_t i = 0; i < 8; i++) {
            *value += (uint64_t)memory[address + i] << i*8;
        }
    }
}

//helper function for validating an address
bool isvalidaddress(y86_reg_t address)
{
    if (address >= 0 && address < MEMSIZE) {
        return true;
    } else {
        return false;
    }
}
