/**
 * Ideas / Extensions:
 * - Arm VM
 * */




// vm memory with 16 bit addresses
uint16_t mem[UINT16_MAX];

enum {
    R_R0 = 0,
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6,
    R_R7,
    R_PC, // program counter
    R_COND,
    R_COUNT
};


// registers
uint16_t reg[R_COUNT]; 

// opcodes
enum {
    OP_BR = 0,  // branch
    OP_ADD,     // add  
    OP_LD,      // load
    OP_ST,      // store
    OP_JSR,     // jump register
    OP_AND,     // bitwise and
    OP_LDR,     // load register
    OP_STR,     // store register
    OP_RTI,     // unused
    OP_NOT,     // bitwise not
    OP_LDI,     // load indirect
    OP_STI,     // store indirect
    OP_JMP,     // jump
    OP_RES,     // reserved
    OP_LEA,     // load effective address    
    OP_TRAP     // execute trap
};

// each instruction is 16 bits; first 4 bits for op code & remaining for parameters


// condition flags
enum {
    FL_POS = 1 << 0,
    FL_ZRO = 1 << 1,
    FL_NEG = 1 << 2
};


uint16_t sign_extend(uint16_t x, int bit_count) {
    if ((x >> (bit_count - 1)) & 1) {
        x |= (0xFFFF << bit_count);
    }
    return x;
}

void update_flags(uint16_t r) {
    if (reg[r] == 0) {
        reg[R_COND] = FL_ZRO;
    }
    else if (reg[r] >> 15) {
        reg[R_COND] = FL_NEG;
    }
    else {
        reg[R_COND] = FL_POS;
    }
}

int main(int argc, const char* argv[]) {
    enum {
        PC_START = 0x3000
    };
    reg[R_PC] = PC_START;
    
    int running = 1;
    while (running) {
        uint16_t instr = mem_read(reg[R_PC]++);               
        uint16_t op = instr >> 12;

        switch (op) {
            case OP_ADD:
                uint16_t r0 = (instr >> 0x9) & 0x7;
                uint16_t r1 = (instr >> 0x6) & 0x7;
               
                reg[r0] = reg[r1] + (instr >> 0x5) & 0x1 ? 
                        sign_extend(instr & 0x1F, 5) : reg[instr & 0x7]; 
                update_flags(r0);
                break;
            case OP_AND:
                uint16_t r0 = (instr >> 0x9) & 0x7;
                uint16_t r1 = (instr >> 0x6) & 0x7;
                
                reg[r0] = reg[r1] & (instr >> 0x5) & 0x1 ? 
                        sign_extend(instr & 0x1F, 5) : reg[instr & 0x7]; 
                update_flags(r0);
                break;
            case OP_NOT:
                uint16_t r0 = (instr >> 0x9) & 0x7;
                uint16_t r1 = (instr >> 0x6) & 0x7;
                reg[r0] = ~reg[r1];
                update_flags(r0);
                break;
            case OP_BR:
                uint16_t n = (instr >> 0x11) & 0x1;
                uint16_t z = (instr >> 0x10) & 0x1;
                uint16_t p = (instr >> 0x9)  & 0x1;
                
                if ((n && reg[R_COND] == FL_NEG) || (z && reg[R_COND] == FL_ZRO)
                           || (p && reg[R_COND] == FL_POS)) {
                    reg[R_PC] += sign_extend(instr & 0x1FF, 9);
                }
                break;
            case OP_JMP:
                reg[R_PC] = (instr >> 0x6) & 0x7;
                break;
            case OP_JSR:
                reg[R_R7] = reg[R_PC];
                if ((instr >> 0x11) & 0x1) reg[R_PC] += sign_extend(instr & 0x7FF, 11);
                else reg[R_PC] = (instr >> 0x6) & 0x7; 
                break;
            case OP_LD:
                uint16_t r0 = reg[(instr >> 0x9) & 0x7];
                reg[r0] = mem[reg[R_PC] + sign_extend(instr & 0x1FF, 9)];
                update_flags(r0);
                break;
            case OP_LDI:
                uint16_t r0 = reg[(instr >> 0x9) & 0x7]; 
                reg[r0] = mem[mem[reg[R_PC] + sign_extend(instr & 0x1FF, 9)]];
                update_flags(r0);
                break;
            case OP_LDR:
                uint16_t r0 = (instr >> 0x9) & 0x7;
                uint16_t r1 = (instr >> 0x6) & 0x7;
                reg[r0] = mem[reg[r1] + sign_extend(instr & 0x3F, 6)];
                update_flags(r0);
                break;
            case OP_LEA:
                uint16_t r0 = (instr >> 0x9) & 0x7;
                reg[r0] = reg[R_PC] + sign_extend(instr & 0x1FF, 9);
                update_flags(r0);
                break;
            case OP_ST:
                mem[reg[R_PC] + sign_extend(instr & 0x1FF)] = reg[(instr >> 0x9) & 0x1FF];
                break;
            case OP_STI:
                uint16_t sr = (instr >> 0x9) & 0x7;
                mem[mem[reg[R_PC] + sign_extend(instr & 0x1FF)]] = sr;
                break;
            case OP_STR:
                break;
            case OP_TRAP:
                break;
            case OP_RES:
            case OP_RTI:
            default:
                break;
        }
    }
}






