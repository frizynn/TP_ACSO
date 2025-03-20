#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "shell.h"

/* 
 * ARMv8 Instruction Types and Masks
 */

// R-type instruction format
typedef struct {
    unsigned int rm : 5;      // Source register 2
    unsigned int shamt : 6;   // Shift amount 
    unsigned int rn : 5;      // Source register 1
    unsigned int rd : 5;      // Destination register
    unsigned int op : 11;     // Opcode
} R_type;

// I-type instruction format
typedef struct {
    unsigned int imm12 : 12;  // Immediate
    unsigned int rn : 5;      // Source register
    unsigned int rd : 5;      // Destination register
    unsigned int op : 10;     // Opcode
} I_type;

// D-type instruction format (load/store)
typedef struct {
    unsigned int offset : 9;  // Address offset
    unsigned int op2 : 2;     // Secondary opcode
    unsigned int rn : 5;      // Base register
    unsigned int rt : 5;      // Target register
    unsigned int op : 11;     // Opcode
} D_type;

// B-type instruction format (branch)
typedef struct {
    int imm26 : 26;           // Branch offset
    unsigned int op : 6;      // Opcode
} B_type;

// CB-type instruction format (compare & branch)
typedef struct {
    int imm19 : 19;           // Branch offset
    unsigned int rt : 5;      // Register to compare
    unsigned int op : 8;      // Opcode
} CB_type;

// Opcode masks for instruction decoding
#define OPCODE_MASK           0xFFFFFFFF  // Full mask
#define R_TYPE_MASK           0x7FE0FC00  // Mask for R-type opcode
#define I_TYPE_MASK           0x7FC00000  // Mask for I-type opcode
#define D_TYPE_MASK           0xFFC00000  // Mask for D-type opcode
#define B_TYPE_MASK           0xFC000000  // Mask for B-type opcode
#define CB_TYPE_MASK          0xFE000000  // Mask for CB-type opcode

// R-type opcodes
#define ADDS_REG              0x2B200000  // ADDS Xd, Xn, Xm
#define SUBS_REG              0x6B200000  // SUBS Xd, Xn, Xm
#define ANDS_REG              0x6A000000  // ANDS Xd, Xn, Xm
#define EOR_REG               0x4A000000  // EOR Xd, Xn, Xm
#define ORR_REG               0x2A000000  // ORR Xd, Xn, Xm
#define MUL                   0x1B007C00  // MUL Xd, Xn, Xm
#define LSL_REG               0x1AC02000  // LSL Xd, Xn, Xm
#define LSR_REG               0x1AC02400  // LSR Xd, Xn, Xm
#define BR                    0xD61F0000  // BR Xn

// I-type opcodes
#define ADDS_IMM              0x31000000  // ADDS Xd, Xn, #imm
#define SUBS_IMM              0x71000000  // SUBS Xd, Xn, #imm
#define ADD_IMM               0x11000000  // ADD Xd, Xn, #imm
#define MOVZ                  0x52800000  // MOVZ Xd, #imm
#define MOVK                  0x72800000  // MOVK Xd, #imm
#define CMP_IMM               0x7100001F  // CMP Xn, #imm

// D-type opcodes
#define LDUR                  0xF8400000  // LDUR Xt, [Xn, #imm]
#define LDURB                 0x38400000  // LDURB Xt, [Xn, #imm]
#define LDURH                 0x78400000  // LDURH Xt, [Xn, #imm]
#define STUR                  0xF8000000  // STUR Xt, [Xn, #imm]
#define STURB                 0x38000000  // STURB Xt, [Xn, #imm]
#define STURH                 0x78000000  // STURH Xt, [Xn, #imm]

// B-type opcodes
#define B                     0x14000000  // B label

// CB-type opcodes
#define CBZ                   0xB4000000  // CBZ Xt, label
#define CBNZ                  0xB5000000  // CBNZ Xt, label

// Conditional Branch opcodes
#define B_COND                0x54000000  // B.cond label
#define COND_EQ               0x0         // Equal (Z=1)
#define COND_NE               0x1         // Not equal (Z=0)
#define COND_LT               0xB         // Less than (N!=V)
#define COND_GT               0xC         // Greater than (Z=0 && N=V)
#define COND_LE               0xD         // Less than or equal (Z=1 || N!=V)
#define COND_GE               0xA         // Greater than or equal (N=V)

// Special opcodes
#define HLT                   0xD4400000  // HLT

// Helper macros
#define X31                   31          // XZR register number

// Function prototypes
void decode_r_type(uint32_t instruction);
void decode_i_type(uint32_t instruction);
void decode_d_type(uint32_t instruction);
void decode_b_type(uint32_t instruction);
void decode_cb_type(uint32_t instruction);
void decode_b_cond(uint32_t instruction);
int check_condition(int cond);
void update_flags(int64_t result);

void process_instruction()
{
    // Read the current instruction at PC
    uint32_t instruction = mem_read_32(CURRENT_STATE.PC);
    
    // Initialize the next state with the current state
    NEXT_STATE = CURRENT_STATE;
    
    // Increment the PC by default (may be overridden by branch instructions)
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    
    // Special case for halt instruction
    if ((instruction & 0xFFFFFFE0) == HLT) {
        RUN_BIT = 0;
        return;
    }
    
    // Decode and execute the instruction based on its type
    uint32_t opcode = instruction & 0xFC000000;
    
    if ((instruction & 0x1F000000) == 0x1B000000 && 
        (instruction & 0x000003FF) == 0x0000007C) {
        // MUL instruction (special case)
        decode_r_type(instruction);
    } else if ((instruction & B_TYPE_MASK) == B) {
        // B instruction
        decode_b_type(instruction);
    } else if ((instruction & CB_TYPE_MASK) == CBZ || 
              (instruction & CB_TYPE_MASK) == CBNZ) {
        // CBZ/CBNZ instructions
        decode_cb_type(instruction);
    } else if ((instruction & 0xFE000000) == B_COND) {
        // B.cond instruction
        decode_b_cond(instruction);
    } else if ((instruction & D_TYPE_MASK) == LDUR || 
              (instruction & D_TYPE_MASK) == LDURB || 
              (instruction & D_TYPE_MASK) == LDURH || 
              (instruction & D_TYPE_MASK) == STUR || 
              (instruction & D_TYPE_MASK) == STURB || 
              (instruction & D_TYPE_MASK) == STURH) {
        // Load/Store instructions
        decode_d_type(instruction);
    } else if ((instruction & 0xFFE0FC00) == BR) {
        // BR instruction
        int rn = (instruction >> 5) & 0x1F;
        NEXT_STATE.PC = CURRENT_STATE.REGS[rn];
    } else if ((instruction & I_TYPE_MASK) == ADDS_IMM || 
              (instruction & I_TYPE_MASK) == SUBS_IMM || 
              (instruction & I_TYPE_MASK) == ADD_IMM || 
              (instruction & I_TYPE_MASK) == MOVZ || 
              (instruction & I_TYPE_MASK) == MOVK || 
              (instruction & I_TYPE_MASK) == CMP_IMM) {
        // I-type instructions
        decode_i_type(instruction);
    } else {
        // R-type instructions
        decode_r_type(instruction);
    }
    
    // XZR register is always 0
    NEXT_STATE.REGS[X31] = 0;
}

void decode_r_type(uint32_t instruction)
{
    int rd = (instruction >> 0) & 0x1F;
    int rn = (instruction >> 5) & 0x1F;
    int rm = (instruction >> 16) & 0x1F;
    int64_t op1, op2, result;
    
    op1 = CURRENT_STATE.REGS[rn];
    op2 = CURRENT_STATE.REGS[rm];
    
    if ((instruction & R_TYPE_MASK) == ADDS_REG) {
        // ADDS Xd, Xn, Xm
        result = op1 + op2;
        if (rd != X31) {
            NEXT_STATE.REGS[rd] = result;
        }
        update_flags(result);
    } else if ((instruction & R_TYPE_MASK) == SUBS_REG) {
        // SUBS Xd, Xn, Xm
        result = op1 - op2;
        if (rd != X31) {
            NEXT_STATE.REGS[rd] = result;
        }
        update_flags(result);
    } else if ((instruction & 0x7FE0FC00) == ANDS_REG) {
        // ANDS Xd, Xn, Xm
        result = op1 & op2;
        if (rd != X31) {
            NEXT_STATE.REGS[rd] = result;
        }
        update_flags(result);
    } else if ((instruction & 0x7FE0FC00) == EOR_REG) {
        // EOR Xd, Xn, Xm
        result = op1 ^ op2;
        if (rd != X31) {
            NEXT_STATE.REGS[rd] = result;
        }
    } else if ((instruction & 0x7FE0FC00) == ORR_REG) {
        // ORR Xd, Xn, Xm
        result = op1 | op2;
        if (rd != X31) {
            NEXT_STATE.REGS[rd] = result;
        }
    } else if ((instruction & 0x1F000000) == 0x1B000000 && 
              (instruction & 0x000003FF) == 0x0000007C) {
        // MUL Xd, Xn, Xm
        result = op1 * op2;
        if (rd != X31) {
            NEXT_STATE.REGS[rd] = result;
        }
    } else if ((instruction & 0xFFE0FC00) == LSL_REG) {
        // LSL Xd, Xn, Xm
        int shift = op2 & 0x3F; // Only the bottom 6 bits count
        result = op1 << shift;
        if (rd != X31) {
            NEXT_STATE.REGS[rd] = result;
        }
    } else if ((instruction & 0xFFE0FC00) == LSR_REG) {
        // LSR Xd, Xn, Xm
        int shift = op2 & 0x3F; // Only the bottom 6 bits count
        result = (uint64_t)op1 >> shift;
        if (rd != X31) {
            NEXT_STATE.REGS[rd] = result;
        }
    }
}

void decode_i_type(uint32_t instruction)
{
    int rd = (instruction >> 0) & 0x1F;
    int rn = (instruction >> 5) & 0x1F;
    int imm12 = (instruction >> 10) & 0xFFF;
    int shift = (instruction >> 22) & 0x3;
    int64_t op1, imm, result;
    
    op1 = CURRENT_STATE.REGS[rn];
    
    // Handle shift for ADD/ADDS/SUB/SUBS
    if (shift == 0) {
        imm = imm12;
    } else if (shift == 1) {
        imm = imm12 << 12;
    } else {
        imm = imm12; // Default case
    }
    
    if ((instruction & I_TYPE_MASK) == ADDS_IMM) {
        // ADDS Xd, Xn, #imm
        result = op1 + imm;
        if (rd != X31) {
            NEXT_STATE.REGS[rd] = result;
        }
        update_flags(result);
    } else if ((instruction & I_TYPE_MASK) == SUBS_IMM) {
        // SUBS Xd, Xn, #imm
        result = op1 - imm;
        if (rd != X31) {
            NEXT_STATE.REGS[rd] = result;
        }
        update_flags(result);
    } else if ((instruction & I_TYPE_MASK) == ADD_IMM) {
        // ADD Xd, Xn, #imm
        result = op1 + imm;
        if (rd != X31) {
            NEXT_STATE.REGS[rd] = result;
        }
    } else if ((instruction & I_TYPE_MASK) == MOVZ) {
        // MOVZ Xd, #imm
        // We're only implementing when shift is 0
        if (rd != X31) {
            NEXT_STATE.REGS[rd] = imm12;
        }
    } else if ((instruction & I_TYPE_MASK) == CMP_IMM) {
        // CMP Xn, #imm (SUBS XZR, Xn, #imm)
        result = op1 - imm;
        update_flags(result);
    }
}

void decode_d_type(uint32_t instruction)
{
    int rt = (instruction >> 0) & 0x1F;
    int rn = (instruction >> 5) & 0x1F;
    int offset = (instruction >> 12) & 0x1FF;
    int64_t addr = CURRENT_STATE.REGS[rn] + offset;
    
    if ((instruction & D_TYPE_MASK) == LDUR) {
        // LDUR Xt, [Xn, #offset]
        if (rt != X31) {
            NEXT_STATE.REGS[rt] = (int64_t)(int32_t)mem_read_32(addr);
        }
    } else if ((instruction & D_TYPE_MASK) == LDURB) {
        // LDURB Xt, [Xn, #offset]
        if (rt != X31) {
            NEXT_STATE.REGS[rt] = (uint64_t)(mem_read_32(addr) & 0xFF);
        }
    } else if ((instruction & D_TYPE_MASK) == LDURH) {
        // LDURH Xt, [Xn, #offset]
        if (rt != X31) {
            NEXT_STATE.REGS[rt] = (uint64_t)(mem_read_32(addr) & 0xFFFF);
        }
    } else if ((instruction & D_TYPE_MASK) == STUR) {
        // STUR Xt, [Xn, #offset]
        mem_write_32(addr, (uint32_t)CURRENT_STATE.REGS[rt]);
    } else if ((instruction & D_TYPE_MASK) == STURB) {
        // STURB Xt, [Xn, #offset]
        uint32_t current = mem_read_32(addr);
        uint32_t value = (current & 0xFFFFFF00) | (CURRENT_STATE.REGS[rt] & 0xFF);
        mem_write_32(addr, value);
    } else if ((instruction & D_TYPE_MASK) == STURH) {
        // STURH Xt, [Xn, #offset]
        uint32_t current = mem_read_32(addr);
        uint32_t value = (current & 0xFFFF0000) | (CURRENT_STATE.REGS[rt] & 0xFFFF);
        mem_write_32(addr, value);
    }
}

void decode_b_type(uint32_t instruction)
{
    // B label
    int32_t imm26 = (instruction & 0x3FFFFFF);
    // Sign extend and multiply by 4 to get byte offset
    int64_t offset = ((imm26 << 6) >> 4); // sign extend and x4
    
    // Update PC (PC-relative)
    NEXT_STATE.PC = CURRENT_STATE.PC + offset;
}

void decode_cb_type(uint32_t instruction)
{
    int rt = (instruction >> 0) & 0x1F;
    int32_t imm19 = (instruction >> 5) & 0x7FFFF;
    // Sign extend and multiply by 4 to get byte offset
    int64_t offset = ((imm19 << 13) >> 11); // sign extend and x4
    
    if ((instruction & CB_TYPE_MASK) == CBZ) {
        // CBZ Xt, label
        if (CURRENT_STATE.REGS[rt] == 0) {
            NEXT_STATE.PC = CURRENT_STATE.PC + offset;
        }
    } else if ((instruction & CB_TYPE_MASK) == CBNZ) {
        // CBNZ Xt, label
        if (CURRENT_STATE.REGS[rt] != 0) {
            NEXT_STATE.PC = CURRENT_STATE.PC + offset;
        }
    }
}

void decode_b_cond(uint32_t instruction)
{
    int cond = instruction & 0xF;
    int32_t imm19 = (instruction >> 5) & 0x7FFFF;
    // Sign extend and multiply by 4 to get byte offset
    int64_t offset = ((imm19 << 13) >> 11); // sign extend and x4
    
    if (check_condition(cond)) {
        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
    }
}

int check_condition(int cond)
{
    switch (cond) {
        case COND_EQ: return CURRENT_STATE.FLAG_Z == 1;                    // Equal
        case COND_NE: return CURRENT_STATE.FLAG_Z == 0;                    // Not Equal
        case COND_LT: return CURRENT_STATE.FLAG_N == 1;                    // Less Than (N!=V, but V=0)
        case COND_GT: return CURRENT_STATE.FLAG_Z == 0 && CURRENT_STATE.FLAG_N == 0; // Greater Than (Z=0 && N=V)
        case COND_LE: return CURRENT_STATE.FLAG_Z == 1 || CURRENT_STATE.FLAG_N == 1; // Less Than or Equal
        case COND_GE: return CURRENT_STATE.FLAG_N == 0;                    // Greater Than or Equal
        default: return 0;
    }
}

void update_flags(int64_t result)
{
    // Update Z flag (result is zero)
    NEXT_STATE.FLAG_Z = (result == 0);
    
    // Update N flag (result is negative)
    NEXT_STATE.FLAG_N = ((result >> 63) & 0x1);
}
