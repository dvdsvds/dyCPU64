#pragma once
#include <vector>
#include <cstdint>

class Memory {
    private:
        std::vector<uint8_t> memory;
    public:
        Memory(size_t size) : memory(size, 0) {};

        uint8_t read8(uint64_t addr) const { return memory[addr]; }
        uint16_t read16(uint64_t addr) const { 
            return static_cast<uint16_t>(memory[addr]) | 
                   static_cast<uint16_t>(memory[addr + 1]) << 8; 
        }
        uint32_t read32(uint64_t addr) const { 
            uint32_t result = 0;
            for(int i = 0; i < 4; i++) {
                result |= static_cast<uint32_t>(memory[addr + i]) << (i * 8);
            }
            return result;
        }
        uint64_t read64(uint64_t addr) const { 
            uint64_t result = 0;
            for(int i = 0; i < 8; i++) {
                result |= static_cast<uint64_t>(memory[addr + i]) << (i * 8);
            }
            return result;
        }
        void write8(uint64_t addr, uint8_t value) { memory[addr] = value; }
        void write16(uint64_t addr, uint16_t value) { 
            memory[addr] = value & 0xFF; 
            memory[addr + 1] = (value >> 8) & 0xFF; 
        }
        void write32(uint64_t addr, uint32_t value) { 
            for(int i = 0; i < 4; i++) {
                memory[addr + i] = (value >> i * 8) & 0xFF;
            }
        }
        void write64(uint64_t addr, uint64_t value) { 
            for(int i = 0; i < 8; i++) {
                memory[addr + i] = (value >> i * 8) & 0xFF;
            }
        }
};