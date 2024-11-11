#ifndef CACHE_H
#define CACHE_H

#include <stdbool.h>

#include "types.h"

/**
 * @note This function is given to you, don't edit
 * @param num_line_bits
 * @param num_block_offset_bits
 * @return cache*
 */
cache* initialize_cache(unsigned int num_line_bits,
                        unsigned int num_block_offset_bits);

/**
 * Returns a struct representing addr's tag, line (optional), and block offset
 * bits, given the cache mapping algorithm and simulation parameters
 *
 * @note For concrete examples, see lecture slides regarding Cache Memory,
 * Mapping Algorithms and/or test cases
 * @note See types.h docstrings for cache_mapping_algo and simulation_parameters
 * @note If cma is FA, the line bits are not used. Thus, addr_fields.line is an
* optional_uint
 * @param addr
 * @param cma
 * @param sim_params
 * @return addr_fields
 */
addr_fields extract_fields(unsigned int addr, cache_mapping_algo cma,
                           simulation_parameters sim_params);

/**
 * Copies the entire block of physical memory that addr is a part of to
 * cache_line->block. That is, all values in the block addr is in are copied,
 * not just the value at addr
 *
 * @note For concrete examples, see lecture slides regarding Cache Memory,
 * Mapping Algorithms and/or test cases
 * @note Only called on a cache miss
 * @param addr
 * @param cache_line should be mutated
 * @param phy_memory array that contains values in physical memory (see
 * memory.h#memory_file_to_array)
 * @param block_locations array that stores the starting addresses of each block
 * (see memory.h#determine_block_locations)
 * @param num_block_offset_bits
 */
void copy_phy_memory_block_to_cache_line(unsigned int addr,
                                         cache_line* cache_line,
                                         unsigned int* phy_memory,
                                         unsigned int* block_locations,
                                         unsigned int num_block_offset_bits);

/**
 * Searches the cache for the value corresponding to memory address addr
 *
 * If found, this is a cache hit. The hit count of the cache line is incremented
 * (i.e., cache is mutated)
 *
 * If not found, this is a cache miss, and the physical memory value at addr
 * must be cached. That is, the block containing addr is copied into a cache
 * line (i.e., cache is mutated). This cache line could be in-use already (value
 * replaced) or not in-use (not replaced)
 *
 * The rv contains booleans regarding hit/miss and replacement/no replacement
 *
 * Other details of the algorithm depend on cma (either DM or FA)
 *
 * For FA, if every cache line has been used, use the LFU (least frequently
 * used) replacement strategy to determine which cache line to replace. That is,
 * replace the cache line with the fewest number of hits. In the event of a tie,
 * use the cache line with a lower index in the array of cache lines. To do so,
 * update and use the min_hit_cnt and lfu_line variables. Our implementation
 * updates these variables at the top of the for loop
 *
 * @note For concrete examples, see lecture slides regarding Cache Memory,
 * Mapping Algorithms. The DM and FA algorithms are identical to the examples
 * given in the slides. You may also inspect the input/output files in data/
 * @note Also see README section cache_read for pseudocode
 * @note Use the helper functions extract_fields and
 * copy_phy_memory_block_to_cache_line (only on cache miss)
 * @param cache always mutated (hit or miss), see above
 * @param addr memory address
 * @param cma
 * @param sim_params
 * @param block_locations array that stores the starting addresses of each block
 * (see memory.h#determine_block_locations)
 * @param phy_memory array that contains values in physical memory (see
 * memory.h#memory_file_to_array)
 * @return cache_read_rv (see types.h definition)
 */
cache_read_rv cache_read(cache* cache, unsigned int addr,
                         cache_mapping_algo cma,
                         simulation_parameters sim_params,
                         unsigned int* block_locations,
                         unsigned int* phy_memory);

#endif  // CACHE_H
