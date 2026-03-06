#include <stdexcept>
#include "decoder.hpp"
#include "types.hpp"

ActionCode Decoder::extraction_ac() {
    ActionCode ac = static_cast<ActionCode>((raw_instr >> 26) & 0x3F);
    switch(ac) {
        case ActionCode::RTR:
        case ActionCode::DTR:
        case ActionCode::DVTDR:
        case ActionCode::JTA:
        case ActionCode::CJ:
        case ActionCode::MEM:
        case ActionCode::STR:
            return ac;
        default:
            throw std::invalid_argument("Invalid ActionCode");
    }
}

uint8_t Decoder::extraction_dr() {
    return (raw_instr >> 21) & 0x1F;
}

uint8_t Decoder::extraction_sc(ActionCode ac) {
    switch (ac) {
        case ActionCode::RTR:
            // 20 - 17
            return (raw_instr >> 17) & 0xF;
            break;
        case ActionCode::DTR:
            // 20 - 17
            return (raw_instr >> 17) & 0xF;
            break;  
        case ActionCode::DVTDR:
            // 20 - 18
            return (raw_instr >> 18) & 0x7;
            break;
        case ActionCode::JTA:
            // 24 - 25
            return (raw_instr >> 24) & 0x3;
            break;
        case ActionCode::CJ:
            // 20 - 18
            return (raw_instr >> 18) & 0x7;
            break;
        case ActionCode::MEM:
            // 20 - 17
            return (raw_instr >> 17) & 0xF;
            break;
        case ActionCode::STR:
            // 20 - 16
            return (raw_instr >> 16) & 0x1F;
            break;
        default:
            throw std::invalid_argument("Invalid ActionCode in extraction_sc");
            break;
    }
}

uint8_t Decoder::extraction_sr(ActionCode ac) {
    switch(ac) {
        case ActionCode::RTR:
            // 16 - 12
            return (raw_instr >> 12) & 0x1F;
            break;
        case ActionCode::DTR:
            // 16 - 12
            return (raw_instr >> 12) & 0x1F;
            break;  
        case ActionCode::CJ:
            // 17 - 13
            return (raw_instr >> 13) & 0x1F;
            break;
        case ActionCode::MEM:
            // 16 - 12
            return (raw_instr >> 12) & 0x1F;
            break;
        default:
            throw std::invalid_argument("Invalid ActionCode in extraction_sr");
            break;
    }
}

uint8_t Decoder::extraction_sr2(ActionCode ac) {
    if(!(ac == ActionCode::RTR)) {
        throw std::invalid_argument("Invalid ActionCode in extraction_sr2");
    }
    // 11 - 7
    return (raw_instr >> 7) & 0x1F;
}

int32_t Decoder::extraction_dv(ActionCode ac) {
    switch(ac) {
        case ActionCode::DTR: {
            int32_t dv = raw_instr & 0xFFF;
            if(dv & 0x800) {
                dv |= 0xFFFFF000;
            }
            return dv;
            break;
        }
        case ActionCode::DVTDR: {
            int32_t dv = raw_instr & 0x3FFFF;
            if(dv & 0x20000) {
                dv |= 0xFFFC0000;
            }
            return dv;
            break;
        }
        case ActionCode::JTA: {
            int32_t dv = raw_instr & 0xFFFFFF;
            if(dv & 0x800000) {
                dv |= 0xFF000000;
            }
            return dv;
            break;
        }
        case ActionCode::CJ: {
            int32_t dv = raw_instr & 0x1FFF;
            if(dv & 0x1000) {
                dv |= 0xFFFFE000;
            }
            return dv;
            break;
        }
        case ActionCode::MEM: {
            int32_t dv = raw_instr & 0xFFF;
            if(dv & 0x800) {
                dv |= 0xFFFFF000;
            }
            return dv;
            break;
        }
        default:
            throw std::invalid_argument("Invalid ActionCode in extraction_dv");
            break;
    }
}

uint16_t Decoder::extraction_spare(ActionCode ac) {
    switch(ac) {
        case ActionCode::RTR:
            return raw_instr & 0x7F;
            break;
        case ActionCode::STR:
            return raw_instr & 0xFFFF;
            break;
        default:
            throw std::invalid_argument("Invalid ActionCode in extraction_spare");
            break;
    }
}

Decoder::instr Decoder::decode(uint32_t instr) {
    Decoder::instr inst;
    raw_instr = instr;
    inst.ac = extraction_ac();

    switch(inst.ac) {
        case ActionCode::RTR:
            inst.dr = extraction_dr();
            inst.sc = extraction_sc(inst.ac);
            inst.sr = extraction_sr(inst.ac);
            inst.sr2 = extraction_sr2(inst.ac);
            inst.spare = extraction_spare(inst.ac);
            break;
        case ActionCode::DTR:
            inst.dr = extraction_dr();
            inst.sc = extraction_sc(inst.ac);
            inst.sr = extraction_sr(inst.ac);
            inst.dv = extraction_dv(inst.ac);
            break;
        case ActionCode::DVTDR:
            inst.dr = extraction_dr();
            inst.sc = extraction_sc(inst.ac);
            inst.dv = extraction_dv(inst.ac);
            break;
        case ActionCode::JTA:
            inst.sc = extraction_sc(inst.ac);
            inst.dv = extraction_dv(inst.ac);
            break;
        case ActionCode::CJ:
            inst.dr = extraction_dr();
            inst.sc = extraction_sc(inst.ac);
            inst.sr = extraction_sr(inst.ac);
            inst.dv = extraction_dv(inst.ac);
            break;
        case ActionCode::MEM:
            inst.dr = extraction_dr();
            inst.sc = extraction_sc(inst.ac);
            inst.sr = extraction_sr(inst.ac);
            inst.dv = extraction_dv(inst.ac);
            break;
        case ActionCode::STR:
            inst.dr = extraction_dr();
            inst.sc = extraction_sc(inst.ac);
            inst.spare = extraction_spare(inst.ac);
            break;
        default:
            break;
    }
    return inst;
}