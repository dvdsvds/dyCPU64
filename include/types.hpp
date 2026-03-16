#pragma once
#include <cstdint>

enum class ActionCode : uint8_t {
    RTR   = 0x00,
    DTR   = 0x01,
    DVTDR = 0x02,
    JTA   = 0x03,
    CJ    = 0x04,
    MEM   = 0x05,
    STR   = 0x06,
    CSR   = 0x07
};

// RTR(0x00)
enum class RTR : uint8_t {
    ADD = 0x0,
    SUB = 0x1,
    MUL = 0x2,
    DIV = 0x3,
    AND = 0x4,
    OR  = 0x5,
    XOR = 0x6,
    SLL = 0x7,
    SRL = 0x8,
    SRA = 0x9,
    NOT = 0xA,
};

// DTR(0x01)
enum class DTR : uint8_t {
    ADV   = 0x0,
    SDV   = 0x1,
    MDV   = 0x2,
    DDV   = 0x3,
    ANDV  = 0x4,
    ORDV  = 0x5,
    SLDV  = 0x6,
    SRDV  = 0x7,
    SRADV = 0x8,
};

// DVTDR(0x02)
enum class DVTDR : uint8_t {
    MSET = 0x0,
    IDVU = 0x1,
    ISDV = 0x2,
    IDVM = 0x3,
    IDVS = 0x4,
};

// JTA(0x03)
enum class JTA : uint8_t {
    JDV = 0x0,
    JR  = 0x1
};

// CJ(0x04)
enum class CJ : uint8_t {
    EJDV  = 0x0,
    NJDV  = 0x1,
    GJDV  = 0x2,
    LJDV  = 0x3,
    GEJDV = 0x4,
    LEJDV = 0x5,
};

// MEM(0x05)
enum class MEM : uint8_t {
    LVM8  = 0x0,
    LVM16 = 0x1,
    LVM32 = 0x2,
    LVM64 = 0x3,
    SVM8  = 0x4,
    SVM16 = 0x5,
    SVM32 = 0x6,
    SVM64 = 0x7,
};

// STR(0x06)
enum class STR : uint8_t {
    CLR  = 0x0,
    HALT = 0x1,
    NOP  = 0x2
};

// CSR(0x07)
enum class CSR : uint8_t {
    CSRR   = 0x0,
    CSRW   = 0x1,
    CSRRW  = 0x2,
    CSRSDV = 0x3,
    CSRCDV = 0x4
};