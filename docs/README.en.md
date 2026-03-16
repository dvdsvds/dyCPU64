# dyCPU
![C++](https://img.shields.io/badge/C++-00599C?style=flat&logo=cplusplus&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-064F8C?style=flat&logo=cmake&logoColor=white)

Custom 64-bit CPU emulator.  

32-bit fixed-length instructions, 64-bit registers. RISC-based with some x86 influences, including R0 as a general-purpose register.  

Part of a bottom-up computing stack: **CPU → Compiler → OS → Networking**

한국어 문서: [README KR](../README.md)

---

## Project Structure

```
dyCPU/
├── include/
│   ├── types.hpp        — ActionCode and subcode enums
│   ├── decoder.hpp      — Decoder class + instr struct
│   ├── alu.hpp          — ALU namespace
│   ├── registerFile.hpp — 32x 64-bit general purpose registers
│   ├── memory.hpp       — 8/16/32/64-bit read/write (little-endian)
│   ├── pipline.hpp      — Pipeline register structs (IFID, IDEX, EXMEM, MEMWB)
│   ├── csrFile.hpp      — CSR register file (16x 64-bit)
│   └── cpu.hpp          — CPU class (double-buffered pipeline)
├── src/
│   ├── decoder.cpp      — 8 extraction functions + decode method
│   ├── alu.cpp          — RTR, DTR, DVTDR, CJ operations
│   └── cpu.cpp          — 5-stage pipeline execution
├── main.cpp             — Test code
└── CMakeLists.txt
```

---

## Registers

- 32 general-purpose registers (R0~R31), all 64-bit
- R0 is a general-purpose register, not hardwired zero (x86 influence)
- 16 CSR registers (64-bit)

---

## ISA

32-bit fixed-length instructions. 8 formats:

| Format | Layout |
|--------|--------|
| RTR (0x00) | AC(6) + DR(5) + SC(4) + SR1(5) + SR2(5) + Spare(7) |
| DTR (0x01) | AC(6) + DR(5) + SC(4) + SR(5) + DV(12) |
| DVTDR (0x02) | AC(6) + DR(5) + SC(3) + DV(18) |
| JTA (0x03) | AC(6) + SC(2) + SR(5) + DV(19) |
| CJ (0x04) | AC(6) + DR(5) + SC(3) + SR(5) + DV(13) |
| MEM (0x05) | AC(6) + DR(5) + SC(4) + SR(5) + DV(12) |
| STR (0x06) | AC(6) + DR(5) + SC(5) + Spare(16) |
| CSR (0x07) | AC(6) + DR(5) + SC(3) + SR(5) + CA(4) + Spare(9)/DV(9) |

### Instruction Set

| Format | Instructions |
|--------|-------------|
| RTR | ADD, SUB, MUL, DIV, AND, OR, XOR, SLL, SRL, SRA, NOT |
| DTR | ADV, SDV, MDV, DDV, ANDV, ORDV, SLDV, SRDV, SRADV |
| DVTDR | MSET, IDVU, ISDV, IDVM, IDVS |
| JTA | JDV (absolute), JR (register indirect) |
| CJ | EJDV, NJDV, GJDV, LJDV, GEJDV, LEJDV |
| MEM | LVM8/16/32/64, SVM8/16/32/64 |
| STR | CLR, HALT, NOP, SYSCALL, IRET |
| CSR | CSRR, CSRW, CSRRW, CSRSDV, CSRCDV |

---

## Pipeline

5 stages: **IF → ID → EX → MEM → WB**

- Double-buffered (cur/next pointer swap)
- Data forwarding (EXMEM → IDEX, MEMWB → IDEX)
- Stall handling (RAW hazard, load hazard)
- No branch prediction (flush IF/ID on branch)

---

## CSR Registers

| # | Name | Description |
|---|------|-------------|
| 0 | STATUS | IE(bit0), PIE(bit1), MODE(bit2), PMODE(bit3) |
| 1 | CAUSE | Interrupt/exception cause code |
| 2 | EPC | Return PC on interrupt |
| 3 | TVEC | Interrupt handler base address |
| 4 | IE | Per-type enable mask (SW:bit0, Timer:bit1, Ext:bit2) |
| 5 | IP | Per-type pending flags |
| 6 | SCRATCH | Temporary storage for handlers |
| 7 | TIMER_CNT | Cycle counter (+1 per cycle) |
| 8 | TIMER_CMP | Timer compare value |
| 9 | HARTID | Core ID (fixed 0, read-only) |

---

## Interrupt System

### CAUSE Codes

| Code | Cause |
|------|-------|
| 0 | Software interrupt (SYSCALL) |
| 1 | Timer interrupt |
| 2 | External interrupt |
| 3 | Illegal instruction / Privilege violation |
| 4 | TRAP (not yet implemented) |

### Interrupt Priority

Exception (TRAP, Illegal) > Timer > External > Software

### Supported Interrupts / Exceptions
- **SYSCALL** (CAUSE=0): Software interrupt
- **Timer** (CAUSE=1): Fired when TIMER_CNT >= TIMER_CMP
- **Illegal Instruction** (CAUSE=3): Privilege violation in User mode (HALT, CSR access)

### Interrupt Entry
1. Pipeline flush (IF, ID, EX → bubble)
2. STATUS.PIE = STATUS.IE, STATUS.IE = 0
3. STATUS.PMODE = STATUS.MODE, STATUS.MODE = 1 (Kernel)
4. EPC = return PC
5. CAUSE = cause code
6. PC = TVEC

### IRET
1. STATUS.IE = STATUS.PIE
2. STATUS.MODE = STATUS.PMODE
3. PC = EPC

### User / Kernel Mode
- HALT and CSR instructions in User mode (STATUS.MODE=0) trigger an exception
- STATUS initializes to 0 (User mode, interrupts disabled)

---

## Build

```bash
cmake -S . -B build
cmake --build build
./build/dyCPU
```

---

## License

[MIT License](../LICENSE)

---

## Tests

### Decoder (15 tests)
All 8 formats + DV sign extension + boundary values + invalid AC exception

### Pipeline

| # | Test | Result |
|---|------|--------|
| 1 | MSET + ADD + HALT | ✓ |
| 2 | Sum loop 1~10 (R10=55) | ✓ |
| 3 | Memory 64-bit SVM64/LVM64 | ✓ |
| 4 | JTA absolute jump | ✓ |
| 5 | EJDV conditional branch | ✓ |
| 6 | GJDV conditional branch | ✓ |
| 7 | LJDV conditional branch | ✓ |
| 8 | MEM 8-bit | ✓ |
| 9 | MEM 16-bit | ✓ |
| 10 | MEM 32-bit | ✓ |
| 11 | Nested loop 3×4=12 | ✓ |
| 12 | Bubble sort {5,3,1,4,2}→{1,2,3,4,5} | ✓ |
| 13 | Forwarding stress chain R10=30 | ✓ |
| 14 | Recursive factorial 5!=120 (JR) | ✓ |
| 15 | CSR read/write/set/clear | ✓ |
| 16 | SYSCALL + IRET | ✓ |
| 17 | Timer Interrupt | ✓ |
| 18 | Privilege Check | ✓ |