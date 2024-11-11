#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "types.h"

inline simulation_parameters SIMULATION_PARAMETERS = {
    .num_addr_bits = 8, .num_line_bits = 3, .num_block_offset_bits = 2};

#define MAX_PHY_MEMORY_LENGTH 65536

// For bit_select
#define UNSIGNED_INT_NUM_BITS sizeof(unsigned int) * 8

#endif  // CONSTANTS_H
