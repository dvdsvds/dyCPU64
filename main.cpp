#include <cassert>
#include <iostream>
#include "decoder.hpp"
#include "cpu.hpp"

// 비트 조립 헬퍼
uint32_t rtr(uint8_t ac, uint8_t dr, uint8_t sc, uint8_t sr1, uint8_t sr2, uint8_t spare) {
    return (ac << 26) | (dr << 21) | (sc << 17) | (sr1 << 12) | (sr2 << 7) | spare;
}

uint32_t dtr(uint8_t ac, uint8_t dr, uint8_t sc, uint8_t sr, int32_t dv) {
    return (ac << 26) | (dr << 21) | (sc << 17) | (sr << 12) | (dv & 0xFFF);
}

uint32_t dvtdr(uint8_t ac, uint8_t dr, uint8_t sc, int32_t dv) {
    return (ac << 26) | (dr << 21) | (sc << 18) | (dv & 0x3FFFF);
}

uint32_t jta(uint8_t ac, uint8_t sc, int32_t dv) {
    return (ac << 26) | (sc << 24) | (dv & 0xFFFFFF);
}

uint32_t cj(uint8_t ac, uint8_t dr, uint8_t sc, uint8_t sr, int32_t dv) {
    return (ac << 26) | (dr << 21) | (sc << 18) | (sr << 13) | (dv & 0x1FFF);
}

uint32_t mem(uint8_t ac, uint8_t dr, uint8_t sc, uint8_t sr, int32_t dv) {
    return (ac << 26) | (dr << 21) | (sc << 17) | (sr << 12) | (dv & 0xFFF);
}

uint32_t str(uint8_t ac, uint8_t dr, uint8_t sc, uint16_t spare) {
    return (ac << 26) | (dr << 21) | (sc << 16) | spare;
}

int main() {
    Decoder decoder;

    // ===== 1. RTR: ADD, DR=3, SC=0(ADD), SR1=5, SR2=7, Spare=0 =====
    {
        auto inst = decoder.decode(rtr(0x00, 3, 0, 5, 7, 0));
        assert(inst.ac == ActionCode::RTR);
        assert(inst.dr == 3);
        assert(inst.sc == 0);
        assert(inst.sr == 5);
        assert(inst.sr2 == 7);
        assert(inst.spare == 0);
        // std::cout << "[PASS] RTR ADD\n";
    }

    // ===== 2. RTR: NOT, DR=10, SC=0xA(NOT), SR1=15, SR2=0, Spare=127 =====
    {
        auto inst = decoder.decode(rtr(0x00, 10, 0xA, 15, 0, 127));
        assert(inst.ac == ActionCode::RTR);
        assert(inst.dr == 10);
        assert(inst.sc == 0xA);
        assert(inst.sr == 15);
        assert(inst.sr2 == 0);
        assert(inst.spare == 127);
        // std::cout << "[PASS] RTR NOT\n";
    }

    // ===== 3. DTR: ADV, DR=1, SC=0(ADV), SR=2, DV=100 (양수) =====
    {
        auto inst = decoder.decode(dtr(0x01, 1, 0, 2, 100));
        assert(inst.ac == ActionCode::DTR);
        assert(inst.dr == 1);
        assert(inst.sc == 0);
        assert(inst.sr == 2);
        assert(inst.dv == 100);
        // std::cout << "[PASS] DTR ADV (DV positive)\n";
    }

    // ===== 4. DTR: SDV, DR=4, SC=1(SDV), SR=6, DV=-1 (음수) =====
    {
        auto inst = decoder.decode(dtr(0x01, 4, 1, 6, -1));
        assert(inst.ac == ActionCode::DTR);
        assert(inst.dr == 4);
        assert(inst.sc == 1);
        assert(inst.sr == 6);
        assert(inst.dv == -1);
        // std::cout << "[PASS] DTR SDV (DV negative)\n";
    }

    // ===== 5. DVTDR: MSET, DR=20, SC=0(MSET), DV=12345 (양수) =====
    {
        auto inst = decoder.decode(dvtdr(0x02, 20, 0, 12345));
        assert(inst.ac == ActionCode::DVTDR);
        assert(inst.dr == 20);
        assert(inst.sc == 0);
        assert(inst.dv == 12345);
        // std::cout << "[PASS] DVTDR MSET (DV positive)\n";
    }

    // ===== 6. DVTDR: ISDV, DR=8, SC=2(ISDV), DV=-500 (음수) =====
    {
        auto inst = decoder.decode(dvtdr(0x02, 8, 2, -500));
        assert(inst.ac == ActionCode::DVTDR);
        assert(inst.dr == 8);
        assert(inst.sc == 2);
        assert(inst.dv == -500);
        // std::cout << "[PASS] DVTDR ISDV (DV negative)\n";
    }

    // ===== 7. JTA: JDV, SC=0(JDV), DV=1000 (양수) =====
    {
        auto inst = decoder.decode(jta(0x03, 0, 1000));
        assert(inst.ac == ActionCode::JTA);
        assert(inst.sc == 0);
        assert(inst.dv == 1000);
        // std::cout << "[PASS] JTA JDV (DV positive)\n";
    }

    // ===== 8. JTA: JDV, SC=0, DV=-100 (음수) =====
    {
        auto inst = decoder.decode(jta(0x03, 0, -100));
        assert(inst.ac == ActionCode::JTA);
        assert(inst.sc == 0);
        assert(inst.dv == -100);
        // std::cout << "[PASS] JTA JDV (DV negative)\n";
    }

    // ===== 9. CJ: EJDV, DR=2, SC=0(EJDV), SR=3, DV=50 (양수) =====
    {
        auto inst = decoder.decode(cj(0x04, 2, 0, 3, 50));
        assert(inst.ac == ActionCode::CJ);
        assert(inst.dr == 2);
        assert(inst.sc == 0);
        assert(inst.sr == 3);
        assert(inst.dv == 50);
        // std::cout << "[PASS] CJ EJDV (DV positive)\n";
    }

    // ===== 10. CJ: NJDV, DR=7, SC=1(NJDV), SR=9, DV=-30 (음수) =====
    {
        auto inst = decoder.decode(cj(0x04, 7, 1, 9, -30));
        assert(inst.ac == ActionCode::CJ);
        assert(inst.dr == 7);
        assert(inst.sc == 1);
        assert(inst.sr == 9);
        assert(inst.dv == -30);
        // std::cout << "[PASS] CJ NJDV (DV negative)\n";
    }

    // ===== 11. MEM: LVM, DR=5, SC=0(LVM), SR=10, DV=200 (양수) =====
    {
        auto inst = decoder.decode(mem(0x05, 5, 0, 10, 200));
        assert(inst.ac == ActionCode::MEM);
        assert(inst.dr == 5);
        assert(inst.sc == 0);
        assert(inst.sr == 10);
        assert(inst.dv == 200);
        // std::cout << "[PASS] MEM LVM (DV positive)\n";
    }

    // ===== 12. MEM: SVM, DR=12, SC=1(SVM), SR=3, DV=-50 (음수) =====
    {
        auto inst = decoder.decode(mem(0x05, 12, 1, 3, -50));
        assert(inst.ac == ActionCode::MEM);
        assert(inst.dr == 12);
        assert(inst.sc == 1);
        assert(inst.sr == 3);
        assert(inst.dv == -50);
        // std::cout << "[PASS] MEM SVM (DV negative)\n";
    }

    // ===== 13. STR: CLR, DR=31, SC=0(CLR), Spare=0 =====
    {
        auto inst = decoder.decode(str(0x06, 31, 0, 0));
        assert(inst.ac == ActionCode::STR);
        assert(inst.dr == 31);
        assert(inst.sc == 0);
        assert(inst.spare == 0);
        // std::cout << "[PASS] STR CLR\n";
    }

    // ===== 14. STR: CLR, DR=0, SC=0, Spare=0xFFFF (최대값) =====
    {
        auto inst = decoder.decode(str(0x06, 0, 0, 0xFFFF));
        assert(inst.ac == ActionCode::STR);
        assert(inst.dr == 0);
        assert(inst.sc == 0);
        assert(inst.spare == 0xFFFF);
        // std::cout << "[PASS] STR CLR (Spare max value)\n";
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
        // std::cout << "[PASS] Invalid AC exception\n";
    }

    // ===== CPU 통합 테스트 =====
    // 테스트: R0=10, R1=20, R2=R0+R1, HALT → R2==30
    {
        Cpu cpu(1024);

        std::vector<uint32_t> program = {
            dvtdr(0x02, 0, 0, 10),    // MSET R0, 10
            dvtdr(0x02, 1, 0, 20),    // MSET R1, 20
            rtr(0x00, 2, 0, 0, 1, 0), // ADD R2, R0, R1
            str(0x06, 0, 1, 0),       // HALT
        };

        std::vector<uint32_t> add = {
            str(0x06, 10, 0, 0),
            dvtdr(0x02, 11, 0, 1),
            dvtdr(0x02, 12, 0, 11),
            rtr(0x00, 10, 0, 10, 11, 0),
            dtr(0x01, 11, 0, 11, 1),
            cj(0x04, 11, 1, 12, -8),
            str(0x06, 0, 1, 0)
        };
        // 메모리 테스트
        std::vector<uint32_t> mem_test = {
            dvtdr(0x02, 0, 0, 42),         // MSET R0, 42
            dvtdr(0x02, 1, 0, 0),          // MSET R1, 0 (주소 베이스)
            mem(0x05, 0, 7, 1, 200),       // SVM64 R0, R1, 200
            mem(0x05, 2, 3, 1, 200),       // LVM64 R2, R1, 200
            str(0x06, 0, 1, 0),            // HALT
        };
        // 분기 테스트 (JTA)
        std::vector<uint32_t> jta_test = {
            dvtdr(0x02, 0, 0, 10),         // MSET R0, 10
            jta(0x03, 0, 20),              // JDV 20
            dvtdr(0x02, 0, 0, 99),         // MSET R0, 99 (건너뜀)
            dvtdr(0x02, 0, 0, 77),         // MSET R0, 77 (건너뜀)
            dvtdr(0x02, 0, 0, 55),         // MSET R0, 55 (건너뜀)
            dvtdr(0x02, 1, 0, 20),         // MSET R1, 20 (주소 20, 여기로 점프)
            str(0x06, 0, 1, 0),            // HALT
        };

        // EJDV 테스트: 같으면 점프
        // R0=5, R1=5, 같으니까 점프해서 R2=100 건너뛰고 R2=1
        std::vector<uint32_t> ejdv_test = {
            dvtdr(0x02, 0, 0, 5),          // MSET R0, 5
            dvtdr(0x02, 1, 0, 5),          // MSET R1, 5
            dvtdr(0x02, 2, 0, 0),          // MSET R2, 0
            cj(0x04, 0, 0, 1, 8),          // EJDV R0, R1, 8 (같으면 주소 20으로)
            dvtdr(0x02, 2, 0, 100),        // MSET R2, 100 (건너뜀)
            dvtdr(0x02, 2, 0, 1),          // MSET R2, 1 (여기로 점프)
            str(0x06, 0, 1, 0),            // HALT
        };
        // assert: R2 == 1

        // GJDV 테스트: SR > DR이면 점프
        // R0=3, R1=7, R1>R0이니까 점프
        std::vector<uint32_t> gjdv_test = {
            dvtdr(0x02, 0, 0, 3),          // MSET R0, 3
            dvtdr(0x02, 1, 0, 7),          // MSET R1, 7
            dvtdr(0x02, 2, 0, 0),          // MSET R2, 0
            cj(0x04, 0, 2, 1, 8),          // GJDV R1, R0, 8 (R1>R0이면 주소 20으로)
            dvtdr(0x02, 2, 0, 100),        // MSET R2, 100 (건너뜀)
            dvtdr(0x02, 2, 0, 1),          // MSET R2, 1 (여기로 점프)
            str(0x06, 0, 1, 0),            // HALT
        };
        // assert: R2 == 1

        // LJDV 테스트: SR < DR이면 점프
        // R0=7, R1=3, R1<R0이니까 점프
        std::vector<uint32_t> ljdv_test = {
            dvtdr(0x02, 0, 0, 7),          // MSET R0, 7
            dvtdr(0x02, 1, 0, 3),          // MSET R1, 3
            dvtdr(0x02, 2, 0, 0),          // MSET R2, 0
            cj(0x04, 0, 3, 1, 8),          // LJDV R1, R0, 8 (R1<R0이면 주소 20으로)
            dvtdr(0x02, 2, 0, 100),        // MSET R2, 100 (건너뜀)
            dvtdr(0x02, 2, 0, 1),          // MSET R2, 1 (여기로 점프)
            str(0x06, 0, 1, 0),            // HALT
        };
        // assert: R2 == 1

        // ===== 메모리 크기별 테스트 =====

        // 8비트 테스트
        std::vector<uint32_t> mem8_test = {
            dvtdr(0x02, 0, 0, 255),        // MSET R0, 255
            dvtdr(0x02, 1, 0, 0),          // MSET R1, 0 (주소 베이스)
            mem(0x05, 0, 4, 1, 300),       // SVM8 R0, R1, 300
            mem(0x05, 2, 0, 1, 300),       // LVM8 R2, R1, 300
            str(0x06, 0, 1, 0),            // HALT
        };
        // assert: R2 == 255

        // 16비트 테스트
        std::vector<uint32_t> mem16_test = {
            dvtdr(0x02, 0, 0, 12345),      // MSET R0, 12345
            dvtdr(0x02, 1, 0, 0),          // MSET R1, 0
            mem(0x05, 0, 5, 1, 300),       // SVM16 R0, R1, 300
            mem(0x05, 2, 1, 1, 300),       // LVM16 R2, R1, 300
            str(0x06, 0, 1, 0),            // HALT
        };
        // assert: R2 == 12345

        // 32비트 테스트
        std::vector<uint32_t> mem32_test = {
            dvtdr(0x02, 0, 0, 100000),     // MSET R0, 100000
            dvtdr(0x02, 1, 0, 0),          // MSET R1, 0
            mem(0x05, 0, 6, 1, 300),       // SVM32 R0, R1, 300
            mem(0x05, 2, 2, 1, 300),       // LVM32 R2, R1, 300
            str(0x06, 0, 1, 0),            // HALT
        };
        // assert: R2 == 100000

        cpu.load_program(add);
        cpu.run();

        {
            Cpu cpu(1024);
            cpu.load_program(program);
            cpu.run();
            assert(cpu.read_register(2) == 30);
            std::cout << "[PASS] MSET + ADD + HALT\n";
        }

        // 1~10 합
        {
            Cpu cpu(1024);
            cpu.load_program(add);
            cpu.run();
            assert(cpu.read_register(10) == 55);
            std::cout << "[PASS] 1+2+...+10 = 55\n";
        }

        // 메모리 64비트
        {
            Cpu cpu(1024);
            cpu.load_program(mem_test);
            cpu.run();
            assert(cpu.read_register(2) == 42);
            std::cout << "[PASS] MEM64\n";
        }

        // JTA 분기
        {
            Cpu cpu(1024);
            cpu.load_program(jta_test);
            cpu.run();
            assert(cpu.read_register(0) == 10);
            assert(cpu.read_register(1) == 20);
            std::cout << "[PASS] JTA\n";
        }

        // EJDV
        {
            Cpu cpu(1024);
            cpu.load_program(ejdv_test);
            cpu.run();
            assert(cpu.read_register(2) == 1);
            std::cout << "[PASS] EJDV\n";
        }

        // GJDV
        {
            Cpu cpu(1024);
            cpu.load_program(gjdv_test);
            cpu.run();
            assert(cpu.read_register(2) == 1);
            std::cout << "[PASS] GJDV\n";
        }

        // LJDV
        {
            Cpu cpu(1024);
            cpu.load_program(ljdv_test);
            cpu.run();
            assert(cpu.read_register(2) == 1);
            std::cout << "[PASS] LJDV\n";
        }

        // MEM 8비트
        {
            Cpu cpu(1024);
            cpu.load_program(mem8_test);
            cpu.run();
            assert(cpu.read_register(2) == 255);
            std::cout << "[PASS] MEM8\n";
        }

        // MEM 16비트
        {
            Cpu cpu(1024);
            cpu.load_program(mem16_test);
            cpu.run();
            assert(cpu.read_register(2) == 12345);
            std::cout << "[PASS] MEM16\n";
        }

        // MEM 32비트
        {
            Cpu cpu(1024);
            cpu.load_program(mem32_test);
            cpu.run();
            assert(cpu.read_register(2) == 100000);
            std::cout << "[PASS] MEM32\n";
        }
    }
    std::cout << "\n=== pass all tests ===\n";
    return 0;
}