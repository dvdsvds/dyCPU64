#include "cpu.hpp"

void Cpu::fetch() {
    fetched_instr = memory.read32(pc);
}

void Cpu::decode_stage() {
    decoded = decoder.decode(fetched_instr);
    switch(decoded.ac) {
        case ActionCode::RTR:
            sr_val = registerFile.read(decoded.sr);
            sr2_val = registerFile.read(decoded.sr2);
            break;
        case ActionCode::DTR:
            sr_val = registerFile.read(decoded.sr);
            break;
        case ActionCode::CJ:
            sr_val = registerFile.read(decoded.sr);
            dr_val = registerFile.read(decoded.dr);
            break;
        case ActionCode::MEM:
            sr_val = registerFile.read(decoded.sr);
            switch(static_cast<MEM>(decoded.sc)) {
                case MEM::SVM8:
                case MEM::SVM16:
                case MEM::SVM32:
                case MEM::SVM64:
                    dr_val = registerFile.read(decoded.dr);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

void Cpu::execute() {
    switch(decoded.ac) {
        case ActionCode::RTR:
            alu_result = Alu::execute_rtr(static_cast<RTR>(decoded.sc), sr_val, sr2_val);
            break;
        case ActionCode::DTR:
            alu_result = Alu::execute_dtr(static_cast<DTR>(decoded.sc), sr_val, decoded.dv);
            break;
        case ActionCode::DVTDR:
            alu_result = Alu::execute_dvtdr(static_cast<DVTDR>(decoded.sc), decoded.dv);
            break;
        case ActionCode::JTA:
            branch_taken = true;
            branch_target = decoded.dv;
            break;
        case ActionCode::CJ:
            if(Alu::execute_cj(static_cast<CJ>(decoded.sc), dr_val, sr_val) == true) {
                branch_taken = true;
                branch_target = pc + decoded.dv;
            }
            break;
        case ActionCode::MEM:
            alu_result = sr_val + decoded.dv;
            break;
        case ActionCode::STR:
            switch(static_cast<STR>(decoded.sc)) {
                case STR::CLR:
                    alu_result = 0;
                    break;
                case STR::HALT:
                    halted = true;
                    break;
                case STR::NOP:
                    break;
            }
            break;
        default:
            break;
    }
}

void Cpu::mem_stage() {
    if(decoded.ac == ActionCode::MEM) {
        switch(static_cast<MEM>(decoded.sc)) {
            case MEM::LVM8:
                mem_result = memory.read8(alu_result);
                break;
            case MEM::LVM16:
                mem_result = memory.read16(alu_result);
                break;
            case MEM::LVM32:
                mem_result = memory.read32(alu_result);
                break;
            case MEM::LVM64:
                mem_result = memory.read64(alu_result);
                break;
            case MEM::SVM8:
                memory.write8(alu_result, static_cast<uint8_t>(dr_val));
                break;
            case MEM::SVM16:
                memory.write16(alu_result, static_cast<uint16_t>(dr_val));
                break;
            case MEM::SVM32:
                memory.write32(alu_result, static_cast<uint32_t>(dr_val));
                break;
            case MEM::SVM64:
                memory.write64(alu_result, static_cast<uint64_t>(dr_val));
                break;
            default:
                break;
        }
    }
}

void Cpu::writeback() {
    switch(decoded.ac) {
        case ActionCode::RTR:
        case ActionCode::DTR:
        case ActionCode::DVTDR:
            registerFile.write(decoded.dr, alu_result);
            break;
        case ActionCode::MEM:
            switch(static_cast<MEM>(decoded.sc)) {
                case MEM::LVM8:
                case MEM::LVM16:
                case MEM::LVM32:
                case MEM::LVM64:
                    registerFile.write(decoded.dr, mem_result);
                    break;
                default:
                    break;
            }
            break;
        case ActionCode::STR:
            switch(static_cast<STR>(decoded.sc)) {
                case STR::CLR:
                    registerFile.write(decoded.dr, alu_result);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }

    if(halted) return;
    if(branch_taken == true) {
        pc = branch_target;
    } else {
        pc += 4;
    }
}

void Cpu::step() {
    fetch();
    decode_stage();
    execute();
    mem_stage();
    writeback();

    branch_taken = false;
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