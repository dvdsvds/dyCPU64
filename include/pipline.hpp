#pragma once
#include "decoder.hpp"

struct IFID {
    uint32_t fetched_instr;
    uint64_t pc;
    bool is_valid;
};

struct IDEX {
    Decoder::instr decoded;
    int64_t sr_val;
    int64_t sr2_val;
    int64_t dr_val;
    uint64_t pc;
    bool is_valid;
};

struct EXMEM {
    Decoder::instr decoded;
    int64_t alu_result;
    int64_t dr_val;
    bool branch_taken;
    uint64_t branch_target;
    uint64_t pc;
    bool is_valid;
    bool is_halted;
};

struct MEMWB {
    Decoder::instr decoded;
    int64_t alu_result;
    int64_t mem_result;
    bool is_valid;
    bool is_halted;
};
