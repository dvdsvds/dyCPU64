#pragma once
#include <array>
#include <cstdint>

enum class CSR_ADDR {
    STATUS    = 0,
    CAUSE     = 1,
    EPC       = 2,
    TVEC      = 3,
    IE        = 4,
    IP        = 5,
    SCRATCH   = 6,
    TIMER_CNT = 7,
    TIMER_CMP = 8,
    MARTID    = 9
};

class CsrFile {
    private:
        std::array<int64_t, 16> regs; 
    public:
        CsrFile() : regs{} {}
        int64_t read(uint8_t addr) const { return regs[addr]; }
        void write(uint8_t addr, int64_t value) { regs[addr] = value; }
        void tick_timer() { regs[static_cast<uint8_t>(CSR_ADDR::TIMER_CNT)]++; }
};