// All functions in this file except number_of_blocks are given to you in
// memory.c

#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#include "types.h"

/**
 * Selects a range of bits from num
 *
 * @param num
 * @param start_bit interpreted as most significant bit in the selected range
 * @param end_bit interpreted as least significant bit in the selected range
 * @return number formed by bits of num in the range start_bit to end_bit,
 * inclusive of both, 0-indexed (i.e., the least significant bit is the 0th bit)
 */
unsigned int bit_select(unsigned int num, unsigned int start_bit,
                        unsigned int end_bit);

/**
 * Computes the number of blocks given num_addr_bits and num_block_offset_bits.
 *
 * For example, if num_addr_bits is 8, then number of addresses is 2^8=256.
 * If the number of block offset bits is 2, the block size is 2^2=4. Thus, the
 * address space can be divided into 64 blocks of 4 addresses each.
 *
 * @note Performs input validation
 * @note You have access to functions such as exp2(n) because math.h is included
 * @param num_addr_bits
 * @param num_block_offset_bits
 * @return {.present=true, .value=num_blocks} (success) | {.present=false,
 * .value=any} (failure, inputs are invalid)
 */
optional_uint number_of_blocks(unsigned int num_addr_bits,
                               unsigned int num_block_offset_bits);

/**
 * Reads a text file to initialize the phy_memory array.
 *
 * The file must contain one hex value per line.
 *
 * @param filepath path to file with one hex value per line
 * @param phy_memory array that is initialized with data from file
 * @return number of bits in memory address, based on the number of values read
 */
unsigned int memory_file_to_array(const char* filepath,
                                  unsigned int* phy_memory);

/**
 * Returns an array that stores the starting addresses of each block
 *
 * For example, if num_block_offset_bits is 3, then each block is size 2^3=8.
 * So, the starting addresses of the blocks would be [0, 8, 16, 24, ...].
 *
 * @param num_blocks number of blocks
 * @param num_block_offset_bits number of block offset bits
 * @return array that stores the starting addresses of each block
 */
unsigned int* determine_block_locations(unsigned int num_blocks,
                                        unsigned int num_block_offset_bits);

#endif  // MEMORY_H
