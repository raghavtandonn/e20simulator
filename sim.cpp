/*
CS-UY 2214
sim.cpp

Raghav Tandon - rt2769
*/

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <regex>
#include <cstdlib>

using namespace std;

// Some helpful constant values that we'll be using.
size_t const static NUM_REGS = 8;
size_t const static MEM_SIZE = 1<<13;
size_t const static REG_SIZE = 1<<16;

/*
    Loads an E20 machine code file into the list
    provided by mem. We assume that mem is
    large enough to hold the values in the machine
    code file.

    @param f Open file to read from
    @param mem Array represetnting memory into which to read program
*/

const unsigned GROUP = 0b000;

// unified handler type
typedef void (*InstrHandler)(unsigned instr,
                             unsigned memory[],
                             unsigned registers[],
                             unsigned &programCounter,
                             bool &halt);



void writeRegister(unsigned registers[], unsigned regIndex, unsigned value);

int signExtend7(unsigned imm7);

// dispatcher helpers
void dispatchGroup(unsigned instr,
                   unsigned memory[],
                   unsigned registers[],
                   unsigned &programCounter,
                   bool &halt);
void dispatchOpcode(unsigned instr,
                    unsigned opcode,
                    unsigned memory[],
                    unsigned registers[],
                    unsigned &programCounter,
                    bool &halt);


void handleAdd (unsigned instr, unsigned memory[], unsigned registers[], unsigned &programCounter, bool &halt);
void handleSub (unsigned instr, unsigned memory[], unsigned registers[], unsigned &programCounter, bool &halt);
void handleOr  (unsigned instr, unsigned memory[], unsigned registers[], unsigned &programCounter, bool &halt);
void handleAnd (unsigned instr, unsigned memory[], unsigned registers[], unsigned &programCounter, bool &halt);
void handleSlt (unsigned instr, unsigned memory[], unsigned registers[], unsigned &programCounter, bool &halt);
void handleJr  (unsigned instr, unsigned memory[], unsigned registers[], unsigned &programCounter, bool &halt);
void handleAddi (unsigned instr, unsigned memory[], unsigned registers[], unsigned &programCounter, bool &halt);
void handleLw   (unsigned instr, unsigned memory[], unsigned registers[], unsigned &programCounter, bool &halt);
void handleSw   (unsigned instr, unsigned memory[], unsigned registers[], unsigned &programCounter, bool &halt);
void handleJeq  (unsigned instr, unsigned memory[], unsigned registers[], unsigned &programCounter, bool &halt);
void handleSlti (unsigned instr, unsigned memory[], unsigned registers[], unsigned &programCounter, bool &halt);
void handleJ    (unsigned instr, unsigned memory[], unsigned registers[], unsigned &programCounter, bool &halt);
void handleJal  (unsigned instr, unsigned memory[], unsigned registers[], unsigned &programCounter, bool &halt);




void load_machine_code(ifstream &f, unsigned mem[]) {
    regex machine_code_re("^ram\\[(\\d+)\\] = 16'b(\\d+);.*$");
    size_t expectedaddr = 0;
    string line;
    while (getline(f, line)) {
        smatch sm;
        if (!regex_match(line, sm, machine_code_re)) {
            cerr << "Can't parse line: " << line << endl;
            exit(1);
        }
        size_t addr = stoi(sm[1], nullptr, 10);
        unsigned instr = stoi(sm[2], nullptr, 2);
        if (addr != expectedaddr) {
            cerr << "Memory addresses encountered out of sequence: " << addr << endl;
            exit(1);
        }
        if (addr >= MEM_SIZE) {
            cerr << "Program too big for memory" << endl;
            exit(1);
        }
        expectedaddr ++;
        mem[addr] = instr;
    }
}

/*
    Prints the current state of the simulator, including
    the current program counter, the current register values,
    and the first memquantity elements of memory.

    @param pc The final value of the program counter
    @param regs Final value of all registers
    @param memory Final value of memory
    @param memquantity How many words of memory to dump
*/
void print_state(unsigned pc, unsigned regs[], unsigned memory[], size_t memquantity) {
    cout << setfill(' ');
    cout << "Final state:" << endl;
    cout << "\tpc=" <<setw(5)<< pc << endl;

    for (size_t reg=0; reg<NUM_REGS; reg++)
        cout << "\t$" << reg << "="<<setw(5)<<regs[reg]<<endl;

    cout << setfill('0');
    bool cr = false;
    for (size_t count=0; count<memquantity; count++) {
        cout << hex << setw(4) << memory[count] << " ";
        cr = true;
        if (count % 8 == 7) {
            cout << endl;
            cr = false;
        }
    }
    if (cr)
        cout << endl;
}

void runSim(unsigned memory[], unsigned registers[], unsigned &programCounter);

void runInstruction(unsigned command, unsigned memory[],unsigned registers[],
    unsigned &programCounter, bool &halt);

/**
    Main function
    Takes command-line args as documented below
*/


int main(int argc, char *argv[]) {
    /*
        Parse the command-line arguments
    */
    char *filename = nullptr;
    bool do_help = false;
    bool arg_error = false;
    for (int i=1; i<argc; i++) {
        string arg(argv[i]);
        if (arg.rfind("-",0)==0) {
            if (arg== "-h" || arg == "--help")
                do_help = true;
            else
                arg_error = true;
        } else {
            if (filename == nullptr)
                filename = argv[i];
            else
                arg_error = true;
        }
    }
    /* Display error message if appropriate */
    if (arg_error || do_help || filename == nullptr) {
        cerr << "usage " << argv[0] << " [-h] filename" << endl << endl;
        cerr << "Simulate E20 machine" << endl << endl;
        cerr << "positional arguments:" << endl;
        cerr << "  filename    The file containing machine code, typically with .bin suffix" << endl<<endl;
        cerr << "optional arguments:"<<endl;
        cerr << "  -h, --help  show this help message and exit"<<endl;
        return 1;
    }

    ifstream f(filename);
    if (!f.is_open()) {
        cerr << "Can't open file "<<filename<<endl;
        return 1;
    }
    // TODO: your code here. Load f and parse using load_machine_code
    unsigned memory[MEM_SIZE] = {0};
    unsigned registers[NUM_REGS] = {0};
    unsigned programCounter = 0;
    // initialize PC registers & memory to 0
    load_machine_code(f, memory);
    f.close();
    // TODO: your code here. Do simulation.

    runSim(memory, registers, programCounter);

    // TODO: your code here. print the final state of the simulator before ending, using print_state

    print_state(programCounter, registers, memory, 128);


    return 0;
}
//ra0Eequ6ucie6Jei0koh6phishohm9

// Execute one 16 bit instruction
void runInstruction(unsigned instr,
                    unsigned memory[],
                    unsigned registers[],
                    unsigned &programCounter,
                    bool &halt) {

    unsigned opcode = (instr >> 13) & 0b111;

    if (opcode == GROUP) {
        dispatchGroup(instr, memory, registers, programCounter, halt);
    } else {
        dispatchOpcode(instr, opcode, memory, registers, programCounter, halt);
    }


    for (size_t r = 0; r < NUM_REGS; r++) {
        if (r == 0) {
            registers[r] = 0;
        } else {
            registers[r] = registers[r] & 0xFFFF;
        }
    }

    programCounter = programCounter & 0xFFFF;
}

// Main loop. fetch memory[pc&0x1FFF], runInstruction on it, stop when halt becomes true. Updates programCounter to final pc
void runSim(unsigned memory[], unsigned registers[], unsigned &programCounter) {
        bool halt = false;
        while (!halt) {
            unsigned pcIndex = programCounter & 0x1FFF; // 0x1FFF = 13 bits of 1s
            unsigned instr = memory[pcIndex];
            runInstruction(instr, memory, registers, programCounter, halt);

            programCounter = programCounter & 0xFFFF;
        }
}


void dispatchGroup(unsigned instr,
                   unsigned memory[],
                   unsigned registers[],
                   unsigned &programCounter,
                   bool &halt) {
    static InstrHandler groupTable[16] = {
        handleAdd,
        handleSub,
        handleOr,
        handleAnd,
        handleSlt,
        nullptr,
        nullptr,
        nullptr,
        handleJr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr
    };

    unsigned func = instr & 0xF;
    InstrHandler h = nullptr;
    if (func < 16) {
        h = groupTable[func];
    }

    if (h != nullptr) {
        h(instr, memory, registers, programCounter, halt);
    } else {
        halt = true;
    }
}

void dispatchOpcode(unsigned instr,
                    unsigned opcode,
                    unsigned memory[],
                    unsigned registers[],
                    unsigned &programCounter,
                    bool &halt) {


    static InstrHandler opTable[8] = {
        nullptr,
        handleAddi,
        handleJ,
        handleJal,
        handleLw,
        handleSw,
        handleJeq,
        handleSlti
    };

    InstrHandler h = nullptr;
    if (opcode < 8) {
        h = opTable[opcode];
    }

    if (h != nullptr) {
        h(instr, memory, registers, programCounter, halt);
    } else {
        halt = true;
    }
}


// add command. dst = srcA + srcB and pc++
void handleAdd(unsigned instr,
               unsigned memory[],
               unsigned registers[],
               unsigned &programCounter,
               bool &halt) {

    unsigned srcA = (instr >> 10) & 0b111;
    unsigned srcB = (instr >> 7)  & 0b111;
    unsigned dst  = (instr >> 4)  & 0b111;

    unsigned result = (registers[srcA] + registers[srcB]) & 0xFFFF;
    writeRegister(registers, dst, result);
    programCounter = (programCounter + 1) & 0xFFFF;

    (void)halt;
}

// sub command . dst = srcA - srcB and pc++
void handleSub(unsigned instr,
               unsigned memory[],
               unsigned registers[],
               unsigned &programCounter,
               bool &halt) {

    unsigned srcA = (instr >> 10) & 0b111;
    unsigned srcB = (instr >> 7)  & 0b111;
    unsigned dst  = (instr >> 4)  & 0b111;

    unsigned result = (registers[srcA] - registers[srcB]) & 0xFFFF;

    writeRegister(registers, dst, result);

    programCounter = (programCounter + 1) & 0xFFFF;

    (void)halt;
}

// or; dst = srcA | srcB and pc++
void handleOr(unsigned instr,
              unsigned memory[],
              unsigned registers[],
              unsigned &programCounter,
              bool &halt) {

    unsigned srcA = (instr >> 10) & 0b111;
    unsigned srcB = (instr >> 7)  & 0b111;
    unsigned dst  = (instr >> 4)  & 0b111;

    unsigned result = (registers[srcA] | registers[srcB]) & 0xFFFF;

    writeRegister(registers, dst, result);

    programCounter = (programCounter + 1) & 0xFFFF;

    (void)halt;
}

// and; dst = srcA & srcB and pc++
void handleAnd(unsigned instr,
               unsigned memory[],
               unsigned registers[],
               unsigned &programCounter,
               bool &halt) {

    unsigned srcA = (instr >> 10) & 0b111;
    unsigned srcB = (instr >> 7)  & 0b111;
    unsigned dst  = (instr >> 4)  & 0b111;

    unsigned result = (registers[srcA] & registers[srcB]) & 0xFFFF;

    writeRegister(registers, dst, result);

    programCounter = (programCounter + 1) & 0xFFFF;

    (void)halt;
}
// slt (unsigned). dst=1 if srcA < srcB, else 0 and then pc++
void handleSlt(unsigned instr,
               unsigned memory[],
               unsigned registers[],
               unsigned &programCounter,
               bool &halt) {

    unsigned srcA = (instr >> 10) & 0b111;
    unsigned srcB = (instr >> 7)  & 0b111;
    unsigned dst  = (instr >> 4)  & 0b111;

    unsigned a = registers[srcA] & 0xFFFF;
    unsigned b = registers[srcB] & 0xFFFF;

    unsigned result = (a < b) ? 1u : 0u;

    writeRegister(registers, dst, result);

    programCounter = (programCounter + 1) & 0xFFFF;

    (void)halt;
}

// jr commend. pc = value in given reg
void handleJr(unsigned instr,
              unsigned memory[],
              unsigned registers[],
              unsigned &programCounter,
              bool &halt) {

    unsigned reg = (instr >> 10) & 0b111;

    programCounter = registers[reg] & 0xFFFF;
    (void)memory;
    (void)halt;
}

// addi command, dst = src + signExt(imm7) and pc++
void handleAddi(unsigned instr,
                unsigned memory[],
                unsigned registers[],
                unsigned &programCounter,
                bool &halt) {

    unsigned regSrc = (instr >> 10) & 0b111;
    unsigned regDst = (instr >> 7)  & 0b111;
    unsigned imm7   =  instr        & 0x7F;

    int immSigned = signExtend7(imm7);

    unsigned result = ( (int)registers[regSrc] + immSigned ) & 0xFFFF;

    writeRegister(registers, regDst, result);

    programCounter = (programCounter + 1) & 0xFFFF;

    (void)memory;
    (void)halt;
}

// lw command, dst = memory[(addrReg + signExt(imm7)) & 0x1FFF] and pc++
void handleLw(unsigned instr,
              unsigned memory[],
              unsigned registers[],
              unsigned &programCounter,
              bool &halt) {

    unsigned regAddr = (instr >> 10) & 0b111;
    unsigned regDst  = (instr >> 7)  & 0b111;
    unsigned imm7    =  instr        & 0x7F;

    int offset = signExtend7(imm7);
    unsigned effAddr = ((int)registers[regAddr] + offset) & 0x1FFF;
    unsigned value = memory[effAddr] & 0xFFFF;
    writeRegister(registers, regDst, value);

    programCounter = (programCounter + 1) & 0xFFFF;

    (void)halt;
}

// sw and memory[(addrReg + signExt(imm7)) & 0x1FFF] = src and pc++
void handleSw(unsigned instr,
              unsigned memory[],
              unsigned registers[],
              unsigned &programCounter,
              bool &halt) {

    unsigned regAddr = (instr >> 10) & 0b111;
    unsigned regSrc  = (instr >> 7)  & 0b111;
    unsigned imm7    =  instr        & 0x7F;

    int offset = signExtend7(imm7);
    unsigned effAddr = ((int)registers[regAddr] + offset) & 0x1FFF;
    memory[effAddr] = registers[regSrc] & 0xFFFF;
    programCounter = (programCounter + 1) & 0xFFFF;

    (void)halt;
}

// jeq command; if regA == regB, pc = pc+1+offset7, else pc = pc+1
void handleJeq(unsigned instr,
               unsigned memory[],
               unsigned registers[],
               unsigned &programCounter,
               bool &halt) {

    unsigned regA    = (instr >> 10) & 0b111;
    unsigned regB    = (instr >> 7)  & 0b111;
    unsigned rel7    =  instr        & 0x7F;

    int offset = signExtend7(rel7);

    unsigned nextPC = (programCounter + 1) & 0xFFFF;

    if ( (registers[regA] & 0xFFFF) == (registers[regB] & 0xFFFF) ) {
        programCounter = (nextPC + offset) & 0xFFFF;
    } else {
        programCounter = nextPC;
    }
    (void)memory;
    (void)halt;
}

// slti (unsigned) command. dst=1 if src < signExt(imm7) and pc++
void handleSlti(unsigned instr,
                unsigned memory[],
                unsigned registers[],
                unsigned &programCounter,
                bool &halt) {

    unsigned regSrc = (instr >> 10) & 0b111;
    unsigned regDst = (instr >> 7)  & 0b111;
    unsigned imm7   =  instr        & 0x7F;

    int immSigned = signExtend7(imm7);
    unsigned immAsUnsigned16 = (unsigned)(immSigned & 0xFFFF);

    unsigned lhs = registers[regSrc] & 0xFFFF;

    unsigned result = (lhs < immAsUnsigned16) ? 1u : 0u;
    writeRegister(registers, regDst, result);
    programCounter = (programCounter + 1) & 0xFFFF;

    (void)memory;
    (void)halt;
}

// j. if target == current pc set halt=true
void handleJ(unsigned instr,
             unsigned memory[],
             unsigned registers[],
             unsigned &programCounter,
             bool &halt) {

    unsigned imm13 = instr & 0x1FFF;

    unsigned curr13 = programCounter & 0x1FFF;
    unsigned target13 = imm13 & 0x1FFF;

    if (target13 == curr13) {
        halt = true;
    } else {
        programCounter = target13 & 0xFFFF;
    }

    (void)memory;
    (void)registers;
}

// jal: $7 = pc+1
void handleJal(unsigned instr,
               unsigned memory[],
               unsigned registers[],
               unsigned &programCounter,
               bool &halt) {

    unsigned imm13 = instr & 0x1FFF;
    unsigned retAddr = (programCounter + 1) & 0xFFFF;
    writeRegister(registers, 7, retAddr);

    programCounter = imm13 & 0x1FFF;

    (void)memory;
    (void)halt;
}


// Write value to regIndex unless = to 0
void writeRegister(unsigned registers[], unsigned regIndex, unsigned value) {
    if (regIndex != 0) {
        registers[regIndex] = value & 0xFFFF;
    }
}

// Sign extend imm7 to int
int signExtend7(unsigned imm7) {
    if (imm7 & 0x40) {
        return (int)(imm7 | 0xFFFFFF80);
    } else {
        return (int)(imm7 & 0x7F);
    }
}




