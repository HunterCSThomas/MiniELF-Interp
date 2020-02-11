#ifndef __CS261_Y86__
#define __CS261_Y86__

#include <stdbool.h>
#include <stdint.h>

#define VADDRBITS 12
#define MEMSIZE (1 << VADDRBITS)
#define NUMREGS 15

/* type declarations */
typedef uint8_t  byte_t;        // byte
typedef uint64_t y86_reg_t;     // register
typedef uint64_t address_t;     // address
typedef bool     flag_t;        // CPU flag

/* possible CPU statuses */
typedef enum { AOK = 1, HLT, ADR, INS } y86_stat_t;

/* y86 CPU data storage structure */
typedef struct y86 {

    y86_reg_t reg[NUMREGS];     // 64-bit general-purpose registers

    flag_t zf;                  // zero flag
    flag_t sf;                  // negative flag
    flag_t of;                  // overflow flag

    y86_reg_t pc;               // program counter

    y86_stat_t stat;            // program status

} y86_t;

/* These enums are specified to match the order of the numbers for all Y86
   instructions and operands. As such, they can be used as constants throughout
   the code. */
typedef enum {
    HALT = 0, NOP, CMOV, IRMOVQ, RMMOVQ, MRMOVQ, OPQ, JUMP, CALL, RET, PUSHQ,
    POPQ, IOTRAP, INVALID
} y86_inst_class_t;

typedef enum {
    RRMOVQ = 0, CMOVLE, CMOVL, CMOVE, CMOVNE, CMOVGE, CMOVG, BADCMOV
} y86_cmov_t;

typedef enum {
    ADD = 0, SUB, AND, XOR, BADOP
} y86_op_t;

typedef enum {
    JMP = 0, JLE, JL, JE, JNE, JGE, JG, BADJUMP
} y86_jump_t;

typedef enum {
    CHAROUT = 0, CHARIN, DECOUT, DECIN, STROUT, FLUSH, BADTRAP
} y86_iotrap_t;

typedef enum {
    RAX = 0, RCX, RDX, RBX, RSP, RBP, RSI, RDI,
    R8, R9, R10, R11, R12, R13, R14, NOREG
} y86_regnum_t;

/* Instruction storage structure; use the constants defined in enums above.
   Comments reflect correlated information from y86 ISA sheet. See the sheet
   for more details. */
typedef struct y86_inst {

    // opcode (first byte)

    uint8_t opcode;             // icode:ifun
    y86_inst_class_t icode;     // high-order 4 bits of opcode
                                // low-order 4 bits of opcode:
    y86_cmov_t cmov;            //   ifun       (cmovXX only)
    y86_op_t op;                //   ifun       (OPq only)
    y86_jump_t jump;            //   ifun       (jXX only)
    y86_iotrap_t id;            //   trap id    (iotrap only)

    // registers (second byte, if present based on icode)

    y86_regnum_t ra;            // rA           (high-order 4 bits)
    y86_regnum_t rb;            // rB           (low-order 4 bits)

    // valC (eight bytes, if present, starting at either second or third byte)

    address_t dest;             // Dest         (jXX and call only)
    int64_t v;                  // V            (irmovq only)
    int64_t d;                  // D            (rmmovq and mrmovq only)

    uint8_t size;               // hard-coded for each instruction

} y86_inst_t;

#endif
