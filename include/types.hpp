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
};

// RTR
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

// DTR
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

// DVTDR
enum class DVTDR : uint8_t {
    MSET  = 0x0,
    IDVU  = 0x1,
    ISDV  = 0x2,
    IDVM  = 0x3,
    IDVS  = 0x4,
};

// JTA
enum class JTA : uint8_t {
    JDV = 0x0,
};

// CJ
enum class CJ : uint8_t {
    EJDV  = 0x0,
    NJDV  = 0x1,
    GJDV  = 0x2,
    LJDV  = 0x3,
    GEJDV = 0x4,
    LEJDV = 0x5,
};

// MEM
enum class MEM : uint8_t {
    LVM   = 0x0,
    SVM   = 0x1,
    LVM8  = 0x2,
    SVM8  = 0x3,
    LVM16 = 0x4,
    SVM16 = 0x5,
    LVM32 = 0x6,
    SVM32 = 0x7,
    LVMS  = 0x8,
};

// STR
enum class STR : uint8_t {
    CLR = 0x0,
};