#pragma once

#include "types.hpp"

namespace Alu {
    int64_t execute_rtr(RTR rtr, int64_t rs1, int64_t rs2);
    int64_t execute_dtr(DTR dtr, int64_t rs1, int64_t dv);
    int64_t execute_dvtdr(DVTDR dvtdr, int64_t dv);
};