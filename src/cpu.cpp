#include "cpu.hpp"
#include <iostream>

void Cpu::fetch() {
    if(halt_detected) {
        ifid_next->is_valid = false;
        return;
    }

    ifid_next->fetched_instr = memory.read32(pc);
    ifid_next->pc = pc;
    ifid_next->is_valid = true;
}

void Cpu::decode_stage() {
    if(!ifid_cur->is_valid) return;

    Decoder::instr temp = decoder.decode(ifid_cur->fetched_instr);
    if(idex_cur->is_valid && writes_to_dr(idex_cur->decoded)) {
        bool need_stall = false;

        need_stall = (idex_cur->decoded.dr == temp.sr);
        if(temp.ac == ActionCode::RTR) {
            need_stall = need_stall || (idex_cur->decoded.dr == temp.sr2);
        }
        if(temp.ac == ActionCode::CJ) {
            need_stall = need_stall || (idex_cur->decoded.dr == temp.dr);
        }
        if(temp.ac == ActionCode::MEM && temp.sc >= 4) {
            need_stall = need_stall || (idex_cur->decoded.dr == temp.dr);
        }
        if(need_stall) {
            idex_next->is_valid = false;
            *ifid_next = *ifid_cur;
            stalled = true;
            return;
        }
        
    }
    if(exmem_cur->is_valid && is_load(exmem_cur->decoded) && writes_to_dr(exmem_cur->decoded)) {
        bool load_hazard = (exmem_cur->decoded.dr == temp.sr);
        if(temp.ac == ActionCode::RTR)
            load_hazard = load_hazard || (exmem_cur->decoded.dr == temp.sr2);
        if(temp.ac == ActionCode::CJ)
            load_hazard = load_hazard || (exmem_cur->decoded.dr == temp.dr);
        if(temp.ac == ActionCode::MEM && temp.sc >= 4)
            load_hazard = load_hazard || (exmem_cur->decoded.dr == temp.dr);
        if(load_hazard) {
            idex_next->is_valid = false;
            *ifid_next = *ifid_cur;
            stalled = true;
            return;
        }
    }
    stalled = false;

    idex_next->decoded = decoder.decode(ifid_cur->fetched_instr);
    switch(idex_next->decoded.ac) {
        case ActionCode::RTR:
            idex_next->sr_val = registerFile.read(idex_next->decoded.sr);
            idex_next->sr2_val = registerFile.read(idex_next->decoded.sr2);
            break;
        case ActionCode::DTR:
            idex_next->sr_val = registerFile.read(idex_next->decoded.sr);
            break;
        case ActionCode::CJ:
            idex_next->sr_val = registerFile.read(idex_next->decoded.sr);
            idex_next->dr_val = registerFile.read(idex_next->decoded.dr);
            break;
        case ActionCode::MEM:
            idex_next->sr_val = registerFile.read(idex_next->decoded.sr);
            switch(static_cast<MEM>(idex_next->decoded.sc)) {
                case MEM::SVM8:
                case MEM::SVM16:
                case MEM::SVM32:
                case MEM::SVM64:
                    idex_next->dr_val = registerFile.read(idex_next->decoded.dr);
                    break;
                default:
                    break;
            }
            break;
        case ActionCode::JTA:
            idex_next->sr_val = registerFile.read(idex_next->decoded.sr);
            break;
        case ActionCode::CSR:
            idex_next->sr_val = registerFile.read(idex_next->decoded.sr);
            break;
        default:
            break;
    }

    if(memwb_cur->is_valid && writes_to_dr(memwb_cur->decoded) && memwb_cur->decoded.dr == idex_next->decoded.sr) {
        if(memwb_cur->decoded.ac == ActionCode::MEM) {
            idex_next->sr_val = memwb_cur->mem_result;
        } else {
            idex_next->sr_val = memwb_cur->alu_result;
        }
    }
    if(exmem_cur->is_valid && writes_to_dr(exmem_cur->decoded) && !is_load(exmem_cur->decoded) && exmem_cur->decoded.dr == idex_next->decoded.sr) {
        idex_next->sr_val = exmem_cur->alu_result;
    }
    if(memwb_cur->is_valid && writes_to_dr(memwb_cur->decoded) && memwb_cur->decoded.dr == idex_next->decoded.sr2) {
        if(memwb_cur->decoded.ac == ActionCode::MEM) {
            idex_next->sr2_val = memwb_cur->mem_result;
        } else {
            idex_next->sr2_val = memwb_cur->alu_result;
        }
    }
    if(exmem_cur->is_valid && writes_to_dr(exmem_cur->decoded) && !is_load(exmem_cur->decoded) && exmem_cur->decoded.dr == idex_next->decoded.sr2) {
        idex_next->sr2_val = exmem_cur->alu_result;
    }
    if(memwb_cur->is_valid && writes_to_dr(memwb_cur->decoded) && memwb_cur->decoded.dr == idex_next->decoded.dr) {
        if(memwb_cur->decoded.ac == ActionCode::MEM) {
            idex_next->dr_val = memwb_cur->mem_result;
        } else {
            idex_next->dr_val = memwb_cur->alu_result;
        }
    }
    if(exmem_cur->is_valid && writes_to_dr(exmem_cur->decoded) && !is_load(exmem_cur->decoded) && exmem_cur->decoded.dr == idex_next->decoded.dr) {
        idex_next->dr_val = exmem_cur->alu_result;
    }

    idex_next->pc = ifid_cur->pc;
    idex_next->is_valid = ifid_cur->is_valid;
}

void Cpu::execute() {
    if(!idex_cur->is_valid) return;

    switch(idex_cur->decoded.ac) {
        case ActionCode::RTR:
            exmem_next->alu_result = Alu::execute_rtr(static_cast<RTR>(idex_cur->decoded.sc), idex_cur->sr_val, idex_cur->sr2_val);
            break;
        case ActionCode::DTR:
            exmem_next->alu_result = Alu::execute_dtr(static_cast<DTR>(idex_cur->decoded.sc), idex_cur->sr_val, idex_cur->decoded.dv);
            break;
        case ActionCode::DVTDR:
            exmem_next->alu_result = Alu::execute_dvtdr(static_cast<DVTDR>(idex_cur->decoded.sc), idex_cur->decoded.dv);
            break;
        case ActionCode::JTA:
            exmem_next->branch_taken = true;
            if(static_cast<JTA>(idex_cur->decoded.sc) == JTA::JR) {
                exmem_next->branch_target = idex_cur->sr_val;
            } else {
                exmem_next->branch_target = idex_cur->decoded.dv;
            }
            break;
        case ActionCode::CJ:
            if(Alu::execute_cj(static_cast<CJ>(idex_cur->decoded.sc), idex_cur->dr_val, idex_cur->sr_val) == true) {
                exmem_next->branch_taken = true;
                exmem_next->branch_target = idex_cur->pc + idex_cur->decoded.dv;
            }
            break;
        case ActionCode::MEM:
            exmem_next->alu_result = idex_cur->sr_val + idex_cur->decoded.dv;
            break;
        case ActionCode::STR:
            switch(static_cast<STR>(idex_cur->decoded.sc)) {
                case STR::CLR:
                    exmem_next->alu_result = 0;
                    break;
                case STR::HALT: {
                    int64_t status = csrFile.read(static_cast<uint8_t>(CSR_ADDR::STATUS));
                    bool mode = (status >> 2) & 1;
                    if(!mode) {
                        exmem_next->is_illegal = true;
                        break;
                    }
                    exmem_next->is_halted = true;
                    halt_detected = true;
                    break;
                }
                case STR::NOP:
                    break;
                case STR::SYSCALL:
                    exmem_next->is_syscall = true;
                    break;
                case STR::IRET:
                    exmem_next->is_iret = true;
                    break;
            }
            break;
        case ActionCode::CSR: {
            int64_t status = csrFile.read(static_cast<uint8_t>(CSR_ADDR::STATUS));
            bool mode = (status >> 2) & 1;
            if(!mode) {
                exmem_next->is_illegal = true;
                break;
            }
            switch(static_cast<CSR>(idex_cur->decoded.sc)) {
                case CSR::CSRR:
                    exmem_next->alu_result = csrFile.read(idex_cur->decoded.ca);
                    break;
                case CSR::CSRW:
                    csrFile.write(idex_cur->decoded.ca, idex_cur->sr_val);
                    break;
                case CSR::CSRRW:
                    exmem_next->alu_result = csrFile.read(idex_cur->decoded.ca);
                    csrFile.write(idex_cur->decoded.ca, idex_cur->sr_val);
                    break;
                case CSR::CSRSDV:
                    csrFile.write(idex_cur->decoded.ca, csrFile.read(idex_cur->decoded.ca) | idex_cur->decoded.dv);
                    break;
                case CSR::CSRCDV:
                    csrFile.write(idex_cur->decoded.ca, csrFile.read(idex_cur->decoded.ca) & ~idex_cur->decoded.dv);
                    break;
            }
            break;
        }
        default:
            break;
    }
    exmem_next->pc = idex_cur->pc;
    exmem_next->decoded = idex_cur->decoded;
    exmem_next->dr_val = idex_cur->dr_val;
    exmem_next->is_valid = idex_cur->is_valid;
}

void Cpu::mem_stage() {
    if(!exmem_cur->is_valid) return;

    if(exmem_cur->decoded.ac == ActionCode::MEM) {
        switch(static_cast<MEM>(exmem_cur->decoded.sc)) {
            case MEM::LVM8:
                memwb_next->mem_result = memory.read8(exmem_cur->alu_result);
                break;
            case MEM::LVM16:
                memwb_next->mem_result = memory.read16(exmem_cur->alu_result);
                break;
            case MEM::LVM32:
                memwb_next->mem_result = memory.read32(exmem_cur->alu_result);
                break;
            case MEM::LVM64:
                memwb_next->mem_result = memory.read64(exmem_cur->alu_result);
                break;
            case MEM::SVM8:
                memory.write8(exmem_cur->alu_result, static_cast<uint8_t>(exmem_cur->dr_val));
                break;
            case MEM::SVM16:
                memory.write16(exmem_cur->alu_result, static_cast<uint16_t>(exmem_cur->dr_val));
                break;
            case MEM::SVM32:
                memory.write32(exmem_cur->alu_result, static_cast<uint32_t>(exmem_cur->dr_val));
                break;
            case MEM::SVM64:
                memory.write64(exmem_cur->alu_result, static_cast<uint64_t>(exmem_cur->dr_val));
                break;
            default:
                break;
        }
    }
    memwb_next->decoded = exmem_cur->decoded;
    memwb_next->alu_result = exmem_cur->alu_result;
    memwb_next->is_halted = exmem_cur->is_halted;
    memwb_next->is_valid = exmem_cur->is_valid;
}

void Cpu::writeback() {
    if(!memwb_cur->is_valid) return;

    switch(memwb_cur->decoded.ac) {
        case ActionCode::RTR:
        case ActionCode::DTR:
        case ActionCode::DVTDR:
            registerFile.write(memwb_cur->decoded.dr, memwb_cur->alu_result);
            break;
        case ActionCode::MEM:
            switch(static_cast<MEM>(memwb_cur->decoded.sc)) {
                case MEM::LVM8:
                case MEM::LVM16:
                case MEM::LVM32:
                case MEM::LVM64:
                    registerFile.write(memwb_cur->decoded.dr, memwb_cur->mem_result);
                    break;
                default:
                    break;
            }
            break;
        case ActionCode::STR:
            switch(static_cast<STR>(memwb_cur->decoded.sc)) {
                case STR::CLR:
                    registerFile.write(memwb_cur->decoded.dr, memwb_cur->alu_result);
                    break;
                default:
                    break;
            }
            break;
        case ActionCode::CSR:
            switch(static_cast<CSR>(memwb_cur->decoded.sc)) {
                case CSR::CSRR:
                case CSR::CSRRW:
                    registerFile.write(memwb_cur->decoded.dr, memwb_cur->alu_result);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }

    if(memwb_cur->is_halted) {
        std::cout << "HALT reached WB!" << std::endl;
        halted = true;
        return;
    }
}

void Cpu::step() {
    *ifid_next = {};
    *idex_next = {};
    *exmem_next = {};
    *memwb_next = {};

    fetch();
    decode_stage();
    execute();
    mem_stage();
    writeback();

    if(exmem_next->branch_taken) {
        pc = exmem_next->branch_target;
        ifid_next->is_valid = false;
        idex_next->is_valid = false;
    } else if(!halt_detected && !stalled) {
        pc += 4;
    }

    if(exmem_cur->is_syscall) {
        int64_t status = csrFile.read(static_cast<uint8_t>(CSR_ADDR::STATUS));
        int64_t ie = (status >> 0) & 1;
        status &= ~(1LL << 1);
        status |= (ie << 1);
        status &= ~1LL;
        int64_t mode = (status >> 2) & 1;
        status &= ~(1LL << 3);
        status |= (mode << 3);
        status |= (1LL << 2);
        
        csrFile.write(static_cast<uint8_t>(CSR_ADDR::STATUS), status);
        csrFile.write(static_cast<uint8_t>(CSR_ADDR::CAUSE), 0);
        csrFile.write(static_cast<uint8_t>(CSR_ADDR::EPC), exmem_cur->pc + 4);
        int64_t tvec = csrFile.read(static_cast<uint8_t>(CSR_ADDR::TVEC));

        pc = tvec;

        ifid_next->is_valid = false;
        idex_next->is_valid = false;
        exmem_next->is_valid = false;
    } 

    if(exmem_cur->is_iret) {
        int64_t status = csrFile.read(static_cast<uint8_t>(CSR_ADDR::STATUS));
        int64_t pie = (status >> 1) & 1;
        status &= ~(1LL << 0);
        status |= (pie << 0);
        int64_t pmode = (status >> 3) & 1;
        status &= ~(1LL << 2);
        status |= (pmode << 2);

        csrFile.write(static_cast<uint8_t>(CSR_ADDR::STATUS), status);
        int64_t epc = csrFile.read(static_cast<uint8_t>(CSR_ADDR::EPC));

        pc = epc;

        ifid_next->is_valid = false;
        idex_next->is_valid = false;
        exmem_next->is_valid = false;
    }


    csrFile.tick_timer();
    int64_t timer_cnt = csrFile.read(static_cast<uint8_t>(CSR_ADDR::TIMER_CNT));
    int64_t timer_cmp = csrFile.read(static_cast<uint8_t>(CSR_ADDR::TIMER_CMP));
    if(timer_cnt >= timer_cmp && timer_cmp > 0) {
        int64_t ip = csrFile.read(static_cast<uint8_t>(CSR_ADDR::IP));
        ip |= (1LL << 1);
        csrFile.write(static_cast<uint8_t>(CSR_ADDR::IP), ip);

        int64_t status = csrFile.read(static_cast<uint8_t>(CSR_ADDR::STATUS));
        int64_t ie = csrFile.read(static_cast<uint8_t>(CSR_ADDR::IE));
        if(((status >> 0) & 1) && ((ie >> 1) & 1)) {
            int64_t status_ie = (status >> 0) & 1;
            status &= ~(1LL << 1);
            status |= (status_ie << 1);
            status &= ~1LL;
            int64_t mode = (status >> 2) & 1;
            status &= ~(1LL << 3);
            status |= (mode << 3);
            status |= (1LL << 2);

            csrFile.write(static_cast<uint8_t>(CSR_ADDR::STATUS), status);
            csrFile.write(static_cast<uint8_t>(CSR_ADDR::CAUSE), 1);
            csrFile.write(static_cast<uint8_t>(CSR_ADDR::EPC), exmem_cur->pc);
            int64_t tvec = csrFile.read(static_cast<uint8_t>(CSR_ADDR::TVEC));

            pc = tvec;

            ifid_next->is_valid = false;
            idex_next->is_valid = false;
            exmem_next->is_valid = false;
        }
    }

    if(exmem_cur->is_illegal) {
        int64_t status = csrFile.read(static_cast<uint8_t>(CSR_ADDR::STATUS));
        int64_t ie = (status >> 0) & 1;
        status &= ~(1LL << 1);
        status |= (ie << 1);
        status &= ~1LL;
        int64_t mode = (status >> 2) & 1;
        status &= ~(1LL << 3);
        status |= (mode << 3);
        status |= (1LL << 2);
        
        csrFile.write(static_cast<uint8_t>(CSR_ADDR::STATUS), status);
        csrFile.write(static_cast<uint8_t>(CSR_ADDR::CAUSE), 3);
        csrFile.write(static_cast<uint8_t>(CSR_ADDR::EPC), exmem_cur->pc);
        int64_t tvec = csrFile.read(static_cast<uint8_t>(CSR_ADDR::TVEC));

        pc = tvec;

        ifid_next->is_valid = false;
        idex_next->is_valid = false;
        exmem_next->is_valid = false;
    } 

    std::swap(ifid_cur, ifid_next);
    std::swap(idex_cur, idex_next);
    std::swap(exmem_cur, exmem_next);
    std::swap(memwb_cur, memwb_next);
}

void Cpu::run() {
    while(true) {
        if(halted == true) {
            break;
        } 
        step();
    }
}

void Cpu::load_program(const std::vector<uint32_t>& inst) {
    for(size_t i = 0; i < inst.size(); i++) {
        memory.write32(i * 4, inst[i]);
    }
}