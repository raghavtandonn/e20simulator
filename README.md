# E20 Simulator

This is a software simulator for the E20 architecture from CS-UY 2214.  
It loads a machine code file (`.bin`), simulates execution of that program on a model of the E20 CPU, and prints the final state: program counter, registers `$0`–`$7`, and the first 128 memory cells.

The simulator is implemented in C++ (single file: `sim.cpp`) and follows the rules in the E20 manual:
- 8 general-purpose 16-bit registers (`$0`–`$7`)
- `$0` is hardwired to 0 (writes to `$0` do nothing)
- 16-bit program counter
- 8192 words of memory (13-bit addresses)
- E20 instruction set: arithmetic, memory ops, branches, jumps, jal/jr, etc.

---

## How to build

You just need `g++` (no external libraries).

```bash
g++ -Wall -o sim sim.cpp
