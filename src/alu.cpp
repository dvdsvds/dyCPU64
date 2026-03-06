#include "alu.hpp"

namespace Alu {
    int64_t execute_rtr(RTR rtr, int64_t sr, int64_t sr2) {
        switch (rtr) {
            case RTR::ADD:
                return sr + sr2;
                break;
            case RTR::SUB:
                return sr - sr2;
                break;
            case RTR::MUL:
                return sr * sr2;
                break;
            case RTR::DIV:
                return sr / sr2;
                break;
            case RTR::AND:
                return sr & sr2;
                break;
            case RTR::OR:
                return sr | sr2;
                break;
            case RTR::XOR:
                return sr ^ sr2;
                break; 
            case RTR::SLL:
                return sr << sr2;
                break;
            case RTR::SRL:
                return static_cast<uint64_t>(sr) >> sr2;
                break;
            case RTR::SRA:
                return static_cast<int64_t>(sr) >> sr2;
                break;
            case RTR::NOT:
                return ~sr;
                break;
            default:
                return 0;
                break;
        }
    }
    int64_t execute_dtr(DTR dtr, int64_t sr1, int64_t dv) {
        switch (dtr) {
            case DTR::ADV:
                return sr1 + dv;
                break;
            case DTR::SDV:
                return sr1 - dv;
                break;
            case DTR::MDV:
                return sr1 * dv;
                break;
            case DTR::DDV:
                return sr1 / dv;
                break;
            case DTR::ANDV:
                return sr1 & dv;
                break;
            case DTR::ORDV:
                return sr1 | dv;
                break;
            case DTR::SLDV:
                return sr1 << dv;
                break;
            case DTR::SRDV:
                return static_cast<uint64_t>(sr1) >> dv;
                break;
            case DTR::SRADV:
                return sr1 >> dv;
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
    bool execute_cj(CJ cj, int64_t dr, int64_t sr) {
        switch(cj) {
            case CJ::EJDV:
                return sr == dr;
                break;
            case CJ::NJDV:
                return sr != dr;
                break;
            case CJ::GJDV:
                return sr > dr;
                break;
            case CJ::LJDV:
                return sr < dr;
                break;
            case CJ::GEJDV:
                return sr >= dr;
                break;
            case CJ::LEJDV:
                return sr <= dr;
                break;
            default:
                return false;
                break;
        }
    }
}