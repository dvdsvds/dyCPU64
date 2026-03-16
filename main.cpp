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

uint32_t jta(uint8_t ac, uint8_t sc, uint8_t sr, int32_t dv) {
    return (ac << 26) | (sc << 24) | (sr << 19) | (dv & 0x7FFFF);
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

uint32_t csr(uint8_t ac, uint8_t dr, uint8_t sc, uint8_t sr, uint8_t ca, uint16_t dv) {
    return (ac << 26) | (dr << 21) | (sc << 18) | (sr << 13) | (ca << 9) | (dv & 0x1FF);
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
        std::cout << "sr=" << (int)inst.sr << std::endl;
        uint32_t raw = dtr(0x01, 1, 0, 2, 100);
        std::cout << "raw=0x" << std::hex << raw << std::endl;
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
        auto inst = decoder.decode(jta(0x03, 0, 0, 1000));
        assert(inst.ac == ActionCode::JTA);
        assert(inst.sc == 0);
        assert(inst.dv == 1000);
        // std::cout << "[PASS] JTA JDV (DV positive)\n";
    }

    // ===== 8. JTA: JDV, SC=0, DV=-100 (음수) =====
    {
        auto inst = decoder.decode(jta(0x03, 0, 0, -100));
        assert(inst.ac == ActionCode::JTA);
        assert(inst.sc == 0);
        assert(inst.sr == 0);
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
            jta(0x03, 0, 0, 20),            // JDV 20
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

    {
        std::vector<uint32_t> prog = {
            str(0x06, 10, 0, 0),          // 0:  CLR R10
            dvtdr(0x02, 3, 0, 3),         // 4:  MSET R3, 3
            dvtdr(0x02, 4, 0, 4),         // 8:  MSET R4, 4
            dvtdr(0x02, 5, 0, 1),         // 12: MSET R5, 1
            str(0x06, 1, 0, 0),           // 16: CLR R1
            str(0x06, 2, 0, 0),           // 20: CLR R2
            rtr(0x00, 10, 0, 10, 5, 0),   // 24: ADD R10, R10, R5
            dtr(0x01, 2, 0, 2, 1),        // 28: ADV R2, R2, 1
            cj(0x04, 2, 1, 4, -8),        // 32: NJDV R2, R4, -8     → 24
            dtr(0x01, 1, 0, 1, 1),        // 36: ADV R1, R1, 1
            cj(0x04, 1, 1, 3, -20),       // 40: NJDV R1, R3, -20    → 20
            str(0x06, 0, 1, 0),           // 44: HALT
        };

        Cpu cpu(1024);
        cpu.load_program(prog);
        cpu.run();
        assert(cpu.read_register(10) == 12);
        std::cout << "[PASS] Nested loop 3x4 = 12\n";
    }

    {
        std::vector<uint32_t> bubble_sort = {
            // 시작 주소 초기화
            dvtdr(0x02, 0, 0x0, 0x800),
            // 레지스터 초기화, 메모리에 배열 저장(SVM64)
            dvtdr(0x02, 1, 0x0, 3),
            mem(0x05, 1, 0x7, 0, 0),
            dvtdr(0x02, 1, 0x0, 1),
            mem(0x05, 1, 0x7, 0, 8),
            dvtdr(0x02, 1, 0x0, 5),
            mem(0x05, 1, 0x7, 0, 16),
            dvtdr(0x02, 1, 0x0, 4),
            mem(0x05, 1, 0x7, 0, 24),
            dvtdr(0x02, 1, 0x0, 2),
            mem(0x05, 1, 0x7, 0, 32),
            // 레지스터 1번 초기화(카운터)
            str(0x06, 1, 0x0, 0),
            // i 값 저장 i < 5니까 i = 4(외부 루프 한계점)
            dvtdr(0x02, 2, 0x0, 4),
            // 내부 루프 카운터
            str(0x06, 3, 0x0, 0),
            // 내부 루프 한계점(j) 초기화(n - 1 - i)
            rtr(0x00, 4, 0x1, 2, 1, 0),
            // arr[i], arr[j+1] 로드
            dtr(0x01, 5, 6, 3, 3),
            rtr(0x00, 6, 0x0, 0, 5, 0),
            mem(0x05, 7, 0x3, 6, 0),
            mem(0x05, 8, 0x3, 6, 8),
            // R7 <= R8이면 스왑, 아니면 건너뛰기
            cj(0x04, 8, 0x5, 7, 12),
            mem(0x05, 8, 0x7, 6, 0),
            mem(0x05, 7, 0x7, 6, 8),
            // j++ / 내부 루프 조건 체크, i++ / 외부 루프 조건 체크
            dtr(0x01, 3, 0x0, 3, 1),
            cj(0x04, 3, 0x1, 4, -32),
            dtr(0x01, 1, 0x0, 1, 1),
            cj(0x04, 1, 0x1, 2, -48),
            str(0x06, 0, 0x1, 0),
        };

        Cpu cpu(4096);
        cpu.load_program(bubble_sort);
        cpu.run();
        assert(static_cast<int64_t>(cpu.read_memory64(0x800))      == 1);
        assert(static_cast<int64_t>(cpu.read_memory64(0x800 + 8))  == 2);
        assert(static_cast<int64_t>(cpu.read_memory64(0x800 + 16)) == 3);
        assert(static_cast<int64_t>(cpu.read_memory64(0x800 + 24)) == 4);
        assert(static_cast<int64_t>(cpu.read_memory64(0x800 + 32)) == 5);
        std::cout << "[PASS] Bubble sort {5,3,1,4,2} -> {1,2,3,4,5}\n";
    }

    {
        std::vector<uint32_t> prog = {
            str(0x06, 10, 0, 0),
            dvtdr(0x02, 3, 0, 3),
            dvtdr(0x02, 4, 0, 4),
            dvtdr(0x02, 5, 0, 1),
            str(0x06, 1, 0, 0),
            str(0x06, 2, 0, 0),
            rtr(0x00, 10, 0, 10, 5, 0),
            dtr(0x01, 2, 0, 2, 1),
            cj(0x04, 2, 1, 4, -8),
            dtr(0x01, 1, 0, 1, 1),
            cj(0x04, 1, 1, 3, -20),
            str(0x06, 0, 1, 0),
        };

        Cpu cpu(1024);
        cpu.load_program(prog);
        cpu.run();
        assert(cpu.read_register(10) == 12);
        std::cout << "[PASS] Nested loop 3x4 = 12\n";
    }

    {
        std::vector<uint32_t> prog = {
            dvtdr(0x02, 1, 0, 1),
            dtr(0x01, 2, 0, 1, 1),
            rtr(0x00, 3, 0, 2, 1, 0),
            rtr(0x00, 4, 2, 3, 2, 0),
            rtr(0x00, 5, 1, 4, 1, 0),
            rtr(0x00, 6, 0, 5, 4, 0),
            rtr(0x00, 7, 0, 6, 5, 0),
            rtr(0x00, 8, 2, 7, 2, 0),
            rtr(0x00, 9, 1, 8, 3, 0),
            rtr(0x00, 10, 0, 9, 1, 0),
            str(0x06, 0, 1, 0),
        };

        Cpu cpu(1024);
        cpu.load_program(prog);
        cpu.run();
        assert(cpu.read_register(1) == 1);
        assert(cpu.read_register(2) == 2);
        assert(cpu.read_register(3) == 3);
        assert(cpu.read_register(4) == 6);
        assert(cpu.read_register(5) == 5);
        assert(cpu.read_register(6) == 11);
        assert(cpu.read_register(7) == 16);
        assert(cpu.read_register(8) == 32);
        assert(cpu.read_register(9) == 29);
        assert(cpu.read_register(10) == 30);
        std::cout << "[PASS] Forwarding stress chain R10=30\n";
    }

    {
        std::vector<uint32_t> prog = {
            dvtdr(0x02, 0, 0, 0x1000),    // 0:  MSET R0, 0x1000 (SP)
            dvtdr(0x02, 1, 0, 5),         // 4:  MSET R1, 5 (n)
            dvtdr(0x02, 2, 0, 1),         // 8:  MSET R2, 1 (result)
            dvtdr(0x02, 4, 0, 1),         // 12: MSET R4, 1
            dvtdr(0x02, 3, 0, 80),        // 16: MSET R3, 80 (done)
            jta(0x03, 0, 0, 24),          // 20: JDV 24

            // factorial (24):
            cj(0x04, 4, 5, 1, 52),        // 24: LEJDV R4, R1, +52 → 76
            dtr(0x01, 0, 1, 0, 8),       // 28: SDV R0, R0, -8
            mem(0x05, 3, 7, 0, 0),        // 32: SVM64 R3, [R0+0]
            dtr(0x01, 0, 1, 0, 8),       // 36: SDV R0, R0, -8
            mem(0x05, 1, 7, 0, 0),        // 40: SVM64 R1, [R0+0]
            dtr(0x01, 1, 1, 1, 1),       // 44: SDV R1, R1, -1
            dvtdr(0x02, 3, 0, 56),        // 48: MSET R3, 56
            jta(0x03, 0, 0, 24),          // 52: JDV 24

            // 복귀점 (56):
            mem(0x05, 1, 3, 0, 0),        // 56: LVM64 R1, [R0+0]
            dtr(0x01, 0, 0, 0, 8),        // 60: ADV R0, R0, 8
            mem(0x05, 3, 3, 0, 0),        // 64: LVM64 R3, [R0+0]
            dtr(0x01, 0, 0, 0, 8),        // 68: ADV R0, R0, 8
            rtr(0x00, 2, 2, 2, 1, 0),     // 72: MUL R2, R2, R1
            jta(0x03, 1, 3, 0),           // 76: JR R3

            // done (80):
            str(0x06, 0, 1, 0),           // 80: HALT
        };

        Cpu cpu(8192);
        cpu.load_program(prog);
        cpu.run();
        assert(cpu.read_register(2) == 120);
        std::cout << "[PASS] Recursive factorial 5! = 120\n";
    }

    {
        std::vector<uint32_t> prog = {
            dvtdr(0x02, 1, 0, 42),            // 0:  MSET R1, 42
            csr(0x07, 0, 1, 1, 6, 0),         // 4:  CSRW SR=R1, CA=6(SCRATCH)
            csr(0x07, 2, 0, 0, 6, 0),         // 8:  CSRR DR=R2, CA=6(SCRATCH)
            csr(0x07, 0, 3, 0, 6, 1),         // 12: CSRSDV CA=6, DV=1
            csr(0x07, 3, 0, 0, 6, 0),         // 16: CSRR DR=R3, CA=6
            csr(0x07, 0, 4, 0, 6, 1),         // 20: CSRCDV CA=6, DV=1
            csr(0x07, 4, 0, 0, 6, 0),         // 24: CSRR DR=R4, CA=6
            dvtdr(0x02, 5, 0, 100),           // 28: MSET R5, 100
            csr(0x07, 6, 2, 5, 6, 0),         // 32: CSRRW DR=R6, SR=R5, CA=6
            csr(0x07, 7, 0, 0, 6, 0),         // 36: CSRR DR=R7, CA=6
            str(0x06, 0, 1, 0),               // 40: HALT
        };

        Cpu cpu(1024);
        cpu.load_program(prog);
        cpu.run();
        assert(cpu.read_register(2) == 42);   // CSRW→CSRR
        assert(cpu.read_register(3) == 43);   // 42 | 1 = 43
        assert(cpu.read_register(4) == 42);   // 43 & ~1 = 42
        assert(cpu.read_register(6) == 42);   // CSRRW: 이전 SCRATCH 값
        assert(cpu.read_register(7) == 100);  // CSRRW 후 SCRATCH = 100
        std::cout << "[PASS] CSR read/write/set/clear\n";
    }
    std::cout << "\n=== pass all tests ===\n";
    return 0;
}