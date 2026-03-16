# dyCPU
![C++](https://img.shields.io/badge/C++-00599C?style=flat&logo=cplusplus&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-064F8C?style=flat&logo=cmake&logoColor=white)

커스텀 64비트 CPU 에뮬레이터. 
32비트 고정 길이 명령어, 64비트 레지스터. RISC 기반이나 R0를 범용 레지스터로 사용하는 등 x86 설계 일부 차용.  
바텀업 컴퓨팅 스택의 일부: **CPU → 컴파일러 → OS → 네트워킹**

English documentation: [README EN](docs/README.en.md)

---

## 프로젝트 구조

```
dyCPU/
├── include/
│   ├── types.hpp        — ActionCode, subcode enum 정의
│   ├── decoder.hpp      — Decoder 클래스 + instr 구조체
│   ├── alu.hpp          — ALU namespace 선언
│   ├── registerFile.hpp — 32개 64비트 범용 레지스터
│   ├── memory.hpp       — 8/16/32/64비트 read/write (리틀 엔디안)
│   ├── pipline.hpp      — 파이프라인 레지스터 구조체 (IFID, IDEX, EXMEM, MEMWB)
│   ├── csrFile.hpp      — CSR 레지스터 파일 (16개 64비트)
│   └── cpu.hpp          — CPU 클래스 (더블 버퍼링 방식)
├── src/
│   ├── decoder.cpp      — extraction 함수 8개 + decode 메서드
│   ├── alu.cpp          — RTR, DTR, DVTDR, CJ 연산
│   └── cpu.cpp          — 파이프라인 5단계 실행
├── main.cpp             — 테스트 코드
└── CMakeLists.txt
```

---

## 레지스터

- 범용 레지스터 32개 (R0~R31), 전부 64비트
- R0는 하드와이어드 제로가 아닌 범용 레지스터로 사용 (x86 설계 차용)
- CSR 레지스터 16개 (64비트)

---

## ISA

32비트 고정 길이 명령어. 8가지 포맷:

| 포맷 | 비트 배치 |
|------|-----------|
| RTR (0x00) | AC(6) + DR(5) + SC(4) + SR1(5) + SR2(5) + Spare(7) |
| DTR (0x01) | AC(6) + DR(5) + SC(4) + SR(5) + DV(12) |
| DVTDR (0x02) | AC(6) + DR(5) + SC(3) + DV(18) |
| JTA (0x03) | AC(6) + SC(2) + SR(5) + DV(19) |
| CJ (0x04) | AC(6) + DR(5) + SC(3) + SR(5) + DV(13) |
| MEM (0x05) | AC(6) + DR(5) + SC(4) + SR(5) + DV(12) |
| STR (0x06) | AC(6) + DR(5) + SC(5) + Spare(16) |
| CSR (0x07) | AC(6) + DR(5) + SC(3) + SR(5) + CA(4) + Spare(9)/DV(9) |

### 명령어 목록

| 포맷 | 명령어 |
|------|--------|
| RTR | ADD, SUB, MUL, DIV, AND, OR, XOR, SLL, SRL, SRA, NOT |
| DTR | ADV, SDV, MDV, DDV, ANDV, ORDV, SLDV, SRDV, SRADV |
| DVTDR | MSET, IDVU, ISDV, IDVM, IDVS |
| JTA | JDV (절대 주소), JR (레지스터 간접) |
| CJ | EJDV, NJDV, GJDV, LJDV, GEJDV, LEJDV |
| MEM | LVM8/16/32/64, SVM8/16/32/64 |
| STR | CLR, HALT, NOP, SYSCALL, IRET |
| CSR | CSRR, CSRW, CSRRW, CSRSDV, CSRCDV |

---

## 파이프라인

5단계: **IF → ID → EX → MEM → WB**

- 더블 버퍼링 방식 (cur/next 포인터 swap)
- 데이터 포워딩 (EXMEM → IDEX, MEMWB → IDEX)
- 스톨 처리 (RAW 해저드, 로드 해저드)
- 분기 예측 없음 (분기 시 IF/ID 플러시)

---

## CSR 레지스터

| 번호 | 이름 | 용도 |
|------|------|------|
| 0 | STATUS | IE(비트0), PIE(비트1), MODE(비트2), PMODE(비트3) |
| 1 | CAUSE | 인터럽트/예외 원인 코드 |
| 2 | EPC | 인터럽트 발생 시 복귀 PC |
| 3 | TVEC | 인터럽트 핸들러 베이스 주소 |
| 4 | IE | 종류별 enable 마스크 (SW:비트0, 타이머:비트1, 외부:비트2) |
| 5 | IP | 종류별 pending |
| 6 | SCRATCH | 핸들러 임시 저장 |
| 7 | TIMER_CNT | 사이클 카운터 (매 사이클 +1) |
| 8 | TIMER_CMP | 타이머 비교값 |
| 9 | HARTID | 코어 ID (0 고정, 읽기 전용) |

---

## 인터럽트 시스템

### CAUSE 코드

| 코드 | 원인 |
|------|------|
| 0 | 소프트웨어 인터럽트 (SYSCALL) |
| 1 | 타이머 인터럽트 |
| 2 | 외부 인터럽트 |
| 3 | 잘못된 명령어 / 권한 위반 |
| 4 | TRAP (미구현) |

### 인터럽트 우선순위

예외 (TRAP, 잘못된 명령어) > 타이머 > 외부 > 소프트웨어

### 지원하는 인터럽트/예외
- **SYSCALL** (CAUSE=0): 소프트웨어 인터럽트
- **타이머** (CAUSE=1): TIMER_CNT >= TIMER_CMP 시 발생
- **Illegal Instruction** (CAUSE=3): User 모드에서 권한 위반 (HALT, CSR 접근)

### 인터럽트 진입 동작
1. 파이프라인 플러시 (IF, ID, EX 버블)
2. STATUS.PIE = STATUS.IE, STATUS.IE = 0
3. STATUS.PMODE = STATUS.MODE, STATUS.MODE = 1 (Kernel)
4. EPC = 복귀 PC
5. CAUSE = 원인 코드
6. PC = TVEC

### IRET 동작
1. STATUS.IE = STATUS.PIE
2. STATUS.MODE = STATUS.PMODE
3. PC = EPC

### User/Kernel 모드
- User 모드(STATUS.MODE=0)에서 HALT, CSR 명령어 실행 시 예외 발생
- 시작 시 STATUS는 0 (User 모드, 인터럽트 비활성)

---

## 빌드

```bash
cmake -S . -B build
cmake --build build
./build/dyCPU
```

---

## 라이선스

[MIT License](LICENSE)

---

## 테스트

### 디코더 (15개)
포맷 8가지 + DV 부호 확장 + 경계값 + 잘못된 AC 예외

### 파이프라인

| # | 테스트 | 결과 |
|---|--------|------|
| 1 | MSET + ADD + HALT | ✓ |
| 2 | 1~10 합 루프 (R10=55) | ✓ |
| 3 | 메모리 64비트 SVM64/LVM64 | ✓ |
| 4 | JTA 절대 점프 | ✓ |
| 5 | EJDV 조건 분기 | ✓ |
| 6 | GJDV 조건 분기 | ✓ |
| 7 | LJDV 조건 분기 | ✓ |
| 8 | MEM 8비트 | ✓ |
| 9 | MEM 16비트 | ✓ |
| 10 | MEM 32비트 | ✓ |
| 11 | 중첩 루프 3×4=12 | ✓ |
| 12 | 버블소트 {5,3,1,4,2}→{1,2,3,4,5} | ✓ |
| 13 | 포워딩 스트레스 체인 R10=30 | ✓ |
| 14 | 재귀 팩토리얼 5!=120 (JR 사용) | ✓ |
| 15 | CSR read/write/set/clear | ✓ |
| 16 | SYSCALL + IRET | ✓ |
| 17 | Timer Interrupt | ✓ |
| 18 | Privilege Check | ✓ |