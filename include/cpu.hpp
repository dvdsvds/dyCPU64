#pragma once
#include "alu.hpp"
#include "decoder.hpp"
#include "memory.hpp"
#include "registerFile.hpp"

#include <vector>

class Cpu {
    private:
        uint64_t pc;
        Decoder decoder;
        Memory memory;
        RegisterFile registerFile;

        uint32_t fetched_instr;
        Decoder::instr decoded;
        int64_t sr_val;
        int64_t sr2_val;
        int64_t dr_val;
        int64_t alu_result;
        int64_t mem_result;
        bool branch_taken;
        uint64_t branch_target;

        bool halted;

        void fetch();
        void decode_stage();
        void execute();
        void mem_stage();
        void writeback();

    public:
        Cpu(size_t mem_size) : pc(0), memory(mem_size), fetched_instr(0), 
                            sr_val(0), sr2_val(0), dr_val(0),
                            alu_result(0), mem_result(0),
                            branch_taken(false), branch_target(0), halted(false) {}
        void step();
        void run();
        void load_program(const std::vector<uint32_t>& inst);
        int64_t read_register(uint8_t addr) const { return registerFile.read(addr); }
};