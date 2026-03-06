#pragma once

#include "types.hpp"

namespace Alu {
    int64_t execute_rtr(RTR rtr, int64_t sr1, int64_t sr2);
    int64_t execute_dtr(DTR dtr, int64_t sr1, int64_t dv);
    int64_t execute_dvtdr(DVTDR dvtdr, int64_t dv);
    bool execute_cj(CJ cj, int64_t dr_val, int64_t sr_val);
};