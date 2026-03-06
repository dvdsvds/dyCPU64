#pragma once
#include <cstdint>
#include "types.hpp"

class Decoder {
    private:
        uint32_t raw_instr;

        ActionCode extraction_ac();
        uint8_t extraction_dr();
        uint8_t extraction_sc(ActionCode ac);
        uint8_t extraction_sr(ActionCode ac);
        uint8_t extraction_sr2(ActionCode ac);
        int32_t extraction_dv(ActionCode ac);
        uint16_t extraction_spare(ActionCode ac);
    public:
        struct instr {
            ActionCode ac;
            uint8_t dr;
            uint8_t sc;
            uint8_t sr;
            uint8_t sr2;
            int32_t dv;
            uint16_t spare;
        };

        instr decode(uint32_t instr);
};