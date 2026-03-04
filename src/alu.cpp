#include "alu.hpp"

namespace Alu {
    int64_t execute_rtr(RTR rtr, int64_t rs1, int64_t rs2) {
        switch (rtr) {
            case RTR::ADD:
                return rs1 + rs2;
                break;
            case RTR::SUB:
                return rs1 - rs2;
                break;
            case RTR::MUL:
                return rs1 * rs2;
                break;
            case RTR::DIV:
                return rs1 / rs2;
                break;
            case RTR::AND:
                return rs1 & rs2;
                break;
            case RTR::OR:
                return rs1 | rs2;
                break;
            case RTR::XOR:
                return rs1 ^ rs2;
                break; 
            case RTR::SLL:
                return rs1 << rs2;
                break;
            case RTR::SRL:
                return static_cast<uint64_t>(rs1) >> rs2;
                break;
            case RTR::SRA:
                return static_cast<int64_t>(rs1) >> rs2;
                break;
            case RTR::NOT:
                return ~rs1;
                break;
            default:
                return 0;
                break;
        }
    }
    int64_t execute_dtr(DTR dtr, int64_t rs1, int64_t dv) {
        switch (dtr) {
            case DTR::ADV:
                return rs1 + dv;
                break;
            case DTR::SDV:
                return rs1 - dv;
                break;
            case DTR::MDV:
                return rs1 * dv;
                break;
            case DTR::DDV:
                return rs1 / dv;
                break;
            case DTR::ANDV:
                return rs1 & dv;
                break;
            case DTR::ORDV:
                return rs1 | dv;
                break;
            case DTR::SLDV:
                return rs1 << dv;
                break;
            case DTR::SRDV:
                return static_cast<uint64_t>(rs1) >> dv;
                break;
            case DTR::SRADV:
                return rs1 >> dv;
                break;
            default:
                return 0;
                break;
        }
    }
    int64_t execute_dvtdr(DVTDR dvtdr, int64_t dv) {
        switch (dvtdr) {
            case DVTDR::MSET:
                return dv;
                break;
            case DVTDR::IDVU:
                return static_cast<uint64_t>(dv) << 46;
                break;
            case DVTDR::ISDV:
                if(dv & (1LL << 17)) {
                    return dv | 0xFFFFFFFFFFFC0000LL;
                }
                return dv;
                break;
            case DVTDR::IDVM:
                return static_cast<uint64_t>(dv) << 21;
                break;
            case DVTDR::IDVS:
                return static_cast<uint64_t>(dv & 0xFFF) << ((dv >> 12) & 0x3F);
                break;
            default:
                return 0;
                break;
        }
    }
}