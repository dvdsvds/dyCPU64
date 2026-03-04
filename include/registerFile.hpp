#pragma once
#include <array>
#include <cstdint>

class RegisterFile {
    private:
        std::array<int64_t, 32> regs; 
    public:
        RegisterFile() : regs{} {}
        int64_t read(uint8_t addr) const { return regs[addr]; }
        void write(uint8_t addr, int64_t value) { regs[addr] = value; }
};