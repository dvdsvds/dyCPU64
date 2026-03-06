#include <cassert>
#include <iostream>
#include "decoder.hpp"

// 비트 조립 헬퍼
uint32_t make_rtr(uint8_t ac, uint8_t dr, uint8_t sc, uint8_t sr1, uint8_t sr2, uint8_t spare) {
    return (ac << 26) | (dr << 21) | (sc << 17) | (sr1 << 12) | (sr2 << 7) | spare;
}

uint32_t make_dtr(uint8_t ac, uint8_t dr, uint8_t sc, uint8_t sr, int32_t dv) {
    return (ac << 26) | (dr << 21) | (sc << 17) | (sr << 12) | (dv & 0xFFF);
}

uint32_t make_dvtdr(uint8_t ac, uint8_t dr, uint8_t sc, int32_t dv) {
    return (ac << 26) | (dr << 21) | (sc << 18) | (dv & 0x3FFFF);
}

uint32_t make_jta(uint8_t ac, uint8_t sc, int32_t dv) {
    return (ac << 26) | (sc << 24) | (dv & 0xFFFFFF);
}

uint32_t make_cj(uint8_t ac, uint8_t dr, uint8_t sc, uint8_t sr, int32_t dv) {
    return (ac << 26) | (dr << 21) | (sc << 18) | (sr << 13) | (dv & 0x1FFF);
}

uint32_t make_mem(uint8_t ac, uint8_t dr, uint8_t sc, uint8_t sr, int32_t dv) {
    return (ac << 26) | (dr << 21) | (sc << 17) | (sr << 12) | (dv & 0xFFF);
}

uint32_t make_str(uint8_t ac, uint8_t dr, uint8_t sc, uint16_t spare) {
    return (ac << 26) | (dr << 21) | (sc << 16) | spare;
}

int main() {
    Decoder decoder;

    // ===== 1. RTR: ADD, DR=3, SC=0(ADD), SR1=5, SR2=7, Spare=0 =====
    {
        auto inst = decoder.decode(make_rtr(0x00, 3, 0, 5, 7, 0));
        assert(inst.ac == ActionCode::RTR);
        assert(inst.dr == 3);
        assert(inst.sc == 0);
        assert(inst.sr == 5);
        assert(inst.sr2 == 7);
        assert(inst.spare == 0);
        std::cout << "[PASS] RTR ADD\n";
    }

    // ===== 2. RTR: NOT, DR=10, SC=0xA(NOT), SR1=15, SR2=0, Spare=127 =====
    {
        auto inst = decoder.decode(make_rtr(0x00, 10, 0xA, 15, 0, 127));
        assert(inst.ac == ActionCode::RTR);
        assert(inst.dr == 10);
        assert(inst.sc == 0xA);
        assert(inst.sr == 15);
        assert(inst.sr2 == 0);
        assert(inst.spare == 127);
        std::cout << "[PASS] RTR NOT\n";
    }

    // ===== 3. DTR: ADV, DR=1, SC=0(ADV), SR=2, DV=100 (양수) =====
    {
        auto inst = decoder.decode(make_dtr(0x01, 1, 0, 2, 100));
        assert(inst.ac == ActionCode::DTR);
        assert(inst.dr == 1);
        assert(inst.sc == 0);
        assert(inst.sr == 2);
        assert(inst.dv == 100);
        std::cout << "[PASS] DTR ADV (DV positive)\n";
    }

    // ===== 4. DTR: SDV, DR=4, SC=1(SDV), SR=6, DV=-1 (음수) =====
    {
        auto inst = decoder.decode(make_dtr(0x01, 4, 1, 6, -1));
        assert(inst.ac == ActionCode::DTR);
        assert(inst.dr == 4);
        assert(inst.sc == 1);
        assert(inst.sr == 6);
        assert(inst.dv == -1);
        std::cout << "[PASS] DTR SDV (DV negative)\n";
    }

    // ===== 5. DVTDR: MSET, DR=20, SC=0(MSET), DV=12345 (양수) =====
    {
        auto inst = decoder.decode(make_dvtdr(0x02, 20, 0, 12345));
        assert(inst.ac == ActionCode::DVTDR);
        assert(inst.dr == 20);
        assert(inst.sc == 0);
        assert(inst.dv == 12345);
        std::cout << "[PASS] DVTDR MSET (DV positive)\n";
    }

    // ===== 6. DVTDR: ISDV, DR=8, SC=2(ISDV), DV=-500 (음수) =====
    {
        auto inst = decoder.decode(make_dvtdr(0x02, 8, 2, -500));
        assert(inst.ac == ActionCode::DVTDR);
        assert(inst.dr == 8);
        assert(inst.sc == 2);
        assert(inst.dv == -500);
        std::cout << "[PASS] DVTDR ISDV (DV negative)\n";
    }

    // ===== 7. JTA: JDV, SC=0(JDV), DV=1000 (양수) =====
    {
        auto inst = decoder.decode(make_jta(0x03, 0, 1000));
        assert(inst.ac == ActionCode::JTA);
        assert(inst.sc == 0);
        assert(inst.dv == 1000);
        std::cout << "[PASS] JTA JDV (DV positive)\n";
    }

    // ===== 8. JTA: JDV, SC=0, DV=-100 (음수) =====
    {
        auto inst = decoder.decode(make_jta(0x03, 0, -100));
        assert(inst.ac == ActionCode::JTA);
        assert(inst.sc == 0);
        assert(inst.dv == -100);
        std::cout << "[PASS] JTA JDV (DV negative)\n";
    }

    // ===== 9. CJ: EJDV, DR=2, SC=0(EJDV), SR=3, DV=50 (양수) =====
    {
        auto inst = decoder.decode(make_cj(0x04, 2, 0, 3, 50));
        assert(inst.ac == ActionCode::CJ);
        assert(inst.dr == 2);
        assert(inst.sc == 0);
        assert(inst.sr == 3);
        assert(inst.dv == 50);
        std::cout << "[PASS] CJ EJDV (DV positive)\n";
    }

    // ===== 10. CJ: NJDV, DR=7, SC=1(NJDV), SR=9, DV=-30 (음수) =====
    {
        auto inst = decoder.decode(make_cj(0x04, 7, 1, 9, -30));
        assert(inst.ac == ActionCode::CJ);
        assert(inst.dr == 7);
        assert(inst.sc == 1);
        assert(inst.sr == 9);
        assert(inst.dv == -30);
        std::cout << "[PASS] CJ NJDV (DV negative)\n";
    }

    // ===== 11. MEM: LVM, DR=5, SC=0(LVM), SR=10, DV=200 (양수) =====
    {
        auto inst = decoder.decode(make_mem(0x05, 5, 0, 10, 200));
        assert(inst.ac == ActionCode::MEM);
        assert(inst.dr == 5);
        assert(inst.sc == 0);
        assert(inst.sr == 10);
        assert(inst.dv == 200);
        std::cout << "[PASS] MEM LVM (DV positive)\n";
    }

    // ===== 12. MEM: SVM, DR=12, SC=1(SVM), SR=3, DV=-50 (음수) =====
    {
        auto inst = decoder.decode(make_mem(0x05, 12, 1, 3, -50));
        assert(inst.ac == ActionCode::MEM);
        assert(inst.dr == 12);
        assert(inst.sc == 1);
        assert(inst.sr == 3);
        assert(inst.dv == -50);
        std::cout << "[PASS] MEM SVM (DV negative)\n";
    }

    // ===== 13. STR: CLR, DR=31, SC=0(CLR), Spare=0 =====
    {
        auto inst = decoder.decode(make_str(0x06, 31, 0, 0));
        assert(inst.ac == ActionCode::STR);
        assert(inst.dr == 31);
        assert(inst.sc == 0);
        assert(inst.spare == 0);
        std::cout << "[PASS] STR CLR\n";
    }

    // ===== 14. STR: CLR, DR=0, SC=0, Spare=0xFFFF (최대값) =====
    {
        auto inst = decoder.decode(make_str(0x06, 0, 0, 0xFFFF));
        assert(inst.ac == ActionCode::STR);
        assert(inst.dr == 0);
        assert(inst.sc == 0);
        assert(inst.spare == 0xFFFF);
        std::cout << "[PASS] STR CLR (Spare max value)\n";
    }

    // ===== 15. 잘못된 AC 예외 테스트 =====
    {
        bool caught = false;
        try {
            decoder.decode(0x1C << 26); // AC=0x1C (정의되지 않은 값)
        } catch (const std::invalid_argument&) {
            caught = true;
        }
        assert(caught);
        std::cout << "[PASS] Invalid AC exception\n";
    }

    std::cout << "\n=== pass all tests ===\n";
    return 0;
}