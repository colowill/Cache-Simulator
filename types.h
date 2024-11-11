#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <stddef.h>

// Same optional idea explained in Lab 3 README section Error signaling
typedef struct {
    bool present;
    unsigned int value;
} optional_uint;

// Direct mapping or fully associative
typedef enum { DM, FA } cache_mapping_algo;

/**
 * For DM, a memory address has tag, line, and block offset bits
 *
 * For FA, a memory address has tag and block offset bits
 *
 * We don't need to store num_tag_bits because it can be computed from the given
 * information, specifically
 *
 * For DM, num_tag_bits = num_addr_bits - num_block_offset_bits - num_line_bits
 *
 * For FA, num_tag_bits = num_addr_bits - num_block_offset_bits
 *
 * @note Although in FA, the address doesn't have line bits, num_line_bits
 * is still used to determine how many cache lines there are (arbitrary choice)
 */
typedef struct {
    unsigned int num_addr_bits;
    unsigned int num_line_bits;
    unsigned int num_block_offset_bits;
} simulation_parameters;

/**
 * Concrete values for a memory address's tag, line (optional), and block offset
 * bits
 *
 * In DM, the address has all three, but in FA, the address does not have line
 * bits
 */
typedef struct {
    unsigned int tag;
    optional_uint line;
    unsigned int block_offset;
} addr_fields;

// Initialized in cache.c#initialize_cache
typedef struct {
    /**
     * Similar to valid bit from slides
     * If true, the cache_line contains a value
     * If false, the cache_line is empty
     */
    bool present;
    unsigned int tag;
    int hit_count;
    /**
     * Array of values contained in the cache line, copied from a physical
     * memory block
     *
     * block_size is not stored but can be trivially computed from
     * num_block_offset_bits where necessary
     */
    unsigned int* block;
} cache_line;

/**
 * Similar to ArrayList, pointer cache_line** does not contain information
 * about the size of the array, so must track size in struct
 * @note This is the same as for an ArrayList
 */
typedef struct {
    cache_line** lines;
    unsigned int num_lines;
} cache;

// cache_read returns multiple values, so store all of them in a struct
typedef struct {
    // If input is invalid (e.g., invalid addr), then present=false, and all
    // other values are arbitrary
    bool present;
    // On cache hit, the retrieved value
    // On cache miss, the value from the given memory address that was copied
    // into a cache line (only the value from the given address, not the entire
    // block)
    unsigned int value;
    // Whether there was a cache hit
    bool hit;
    // Whether an existing cache line's block was overwritten (can only be true
    // on a cache miss)
    bool replace;
} cache_read_rv;

#endif  // TYPES_H
