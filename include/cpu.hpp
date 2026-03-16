#pragma once
#include "alu.hpp"
#include "decoder.hpp"
#include "memory.hpp"
#include "registerFile.hpp"
#include "pipline.hpp"
#include "csrFile.hpp"

#include <vector>

class Cpu {
    private:
        uint64_t pc;
        Decoder decoder;
        Memory memory;
        RegisterFile registerFile;
        CsrFile csrFile;

        IFID ifid_a{}, ifid_b{};
        IFID* ifid_cur = &ifid_a;
        IFID* ifid_next = &ifid_b;

        IDEX idex_a{}, idex_b{};
        IDEX* idex_cur = &idex_a;
        IDEX* idex_next = &idex_b;

        EXMEM exmem_a{}, exmem_b{};
        EXMEM* exmem_cur = &exmem_a;
        EXMEM* exmem_next = &exmem_b;

        MEMWB memwb_a{}, memwb_b{};
        MEMWB* memwb_cur = &memwb_a;
        MEMWB* memwb_next = &memwb_b;

        bool halted;
        bool halt_detected;
        bool stalled;

        void fetch();
        void decode_stage();
        void execute();
        void mem_stage();
        void writeback();

        bool writes_to_dr(const Decoder::instr& d) const {
            switch(d.ac) {
                case ActionCode::JTA:  return false;
                case ActionCode::CJ:   return false;
                case ActionCode::MEM:
                    return d.sc <= 3;
                case ActionCode::STR:
                    return d.sc == 0;
                case ActionCode::CSR:
                    return d.sc == 0x0 || d.sc == 0x2;
                default: return true;
            }
        }

        bool is_load(const Decoder::instr& d) {
            return d.ac == ActionCode::MEM && d.sc <= 3;
        }

    public:
        Cpu(size_t mem_size) : pc(0), memory(mem_size), halted(false), halt_detected(false), stalled(false) {}
        void step();
        void run();
        void load_program(const std::vector<uint32_t>& inst);
        int64_t read_register(uint8_t addr) const { return registerFile.read(addr); }

        uint64_t read_memory64(uint64_t addr) {
            return memory.read64(addr);
        }
};