/**
 * See types.h (very important)
 *
 * Also see constants.h#SIMULATION_PARAMETERS for important simulation
 * parameters, i.e. {num_addr_bits = 8, .num_line_bits = 3,
 * .num_block_offset_bits = 2}
 *
 * For simplicity, the simulation uses only these hardcoded values, but some
 * test cases use different values by creating a simulation_parameters struct
 * with different values. If you hardcode the above int literals in your code,
 * you won't pass all test cases. So, you must use the symbolic variable names,
 * and don't hardcode int literals
 */

#include "cache.h"

#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "constants.h"
#include "memory.h"

// This function is given to you, don't edit
cache* initialize_cache(unsigned int num_line_bits,
                        unsigned int num_block_offset_bits) {
    cache* rv = (cache*)malloc(sizeof(cache));

    unsigned int num_lines = exp2(num_line_bits);
    unsigned int block_size = exp2(num_block_offset_bits);

    cache_line** lines = (cache_line**)malloc(sizeof(cache_line*) * num_lines);

    for (size_t line = 0; line < num_lines; line++) {
        lines[line] = (cache_line*)malloc(sizeof(cache_line));
        lines[line]->present = false;  // Cache line is initially empty
        lines[line]->tag = 0xdead;     // Junk value
        lines[line]->hit_count = -1;   // Junk value
        lines[line]->block =
            (unsigned int*)malloc(sizeof(unsigned int) * block_size);
    }

    rv->lines = lines;
    rv->num_lines = num_lines;

    return rv;
}

addr_fields extract_fields(unsigned int addr, cache_mapping_algo cma,
                           simulation_parameters sim_params) {
   
    addr_fields rv = {
        .tag = 0, .line = {.present = false, .value = 0}, .block_offset = 0};

    unsigned int block_mask = (1 << sim_params.num_block_offset_bits) - 1;
    rv.block_offset = addr & block_mask;

    if (cma == DM) {
        unsigned int line_mask = (1 << sim_params.num_line_bits) - 1;
        rv.line.present = true;
        rv.line.value = (addr >> sim_params.num_block_offset_bits) & line_mask;
        rv.tag = addr >> (sim_params.num_block_offset_bits + sim_params.num_line_bits);
    }  else if (cma == FA) {
        rv.tag = addr >> sim_params.num_block_offset_bits;
    }

    return rv;
}

void copy_phy_memory_block_to_cache_line(unsigned int addr,
                                         cache_line* cache_line,
                                         unsigned int* phy_memory,
                                         unsigned int* block_locations,
                                         unsigned int num_block_offset_bits) {
    unsigned int block_size = 1 << num_block_offset_bits;

    // Get the index of the block from the block_locations array
    unsigned int block_index = addr / block_size;  // assuming block size is in bytes

    // Use block_locations to get the starting address of the block
    unsigned int block_start_add = block_locations[block_index];

    // Copy the block from physical memory to cache
    for (unsigned int i = 0; i < block_size; i++) {
        cache_line->block[i] = phy_memory[block_start_add + i];
    }
    cache_line->present = true;    

}

cache_read_rv cache_read(cache* cache, unsigned int addr,
                         cache_mapping_algo cma,
                         simulation_parameters sim_params,
                         unsigned int* block_locations,
                         unsigned int* phy_memory) {
    cache_read_rv rv = {
        .present = false, .value = 0xdead, .hit = false, .replace = false};

    // Input validation
    if (addr >= (unsigned int)exp2(sim_params.num_addr_bits)) {
        // present = false
        return rv;
    }

    rv.present = true;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
    cache_line** lines = cache->lines;

    addr_fields fields = extract_fields(addr, cma, sim_params);
    unsigned int tag = fields.tag;
    unsigned int line;
    // If cma == FA, line is not part of the address's fields
    if (fields.line.present) line = fields.line.value;
    unsigned int block_offset = fields.block_offset;

    // For FA, keeps track of hit count of LFU cache line
    int min_hit_cnt = INT_MAX;
    // For FA, keeps track of index of LFU cache line
    int lfu_line = 0;
#pragma GCC diagnostic pop


   switch (cma) {
        case DM: {
            cache_line* ptr = cache->lines[line];
            if (!fields.line.present) return rv;
            if (ptr->present == true && ptr->tag == fields.tag) {
                ptr->hit_count++;
                rv.hit = true;
                rv.value = ptr->block[fields.block_offset];
            } else {
                rv.hit = false;
                rv.replace = ptr->present;
                ptr->tag = fields.tag;
                ptr->hit_count = 1;
                ptr->present = true;
                copy_phy_memory_block_to_cache_line(addr, ptr, phy_memory, block_locations, sim_params      .num_block_offset_bits);
                rv.value = ptr->block[fields.block_offset];
            } 
            break;
        }
        case FA: {
            cache_line* ptr = NULL;
            bool found = false;
            for (unsigned int i = 0; i < cache->num_lines; i++) {
            if (cache->lines[i]->present && cache->lines[i]->tag == fields.tag) {
                ptr = cache->lines[i];
                ptr->hit_count++;
                rv.hit = true;
                rv.value = ptr->block[fields.block_offset];
                found = true;
                break;
            }
        }

            if (!found) {
                for (unsigned int i = 0; i < cache->num_lines; i++) {
                    if (!cache->lines[i]->present) {
                        ptr = cache->lines[i];
                        break;
                    }
                }

            if (ptr == NULL) {
                ptr = cache->lines[0];
                for (unsigned int i = 1; i < cache->num_lines; i++) {
                    if (cache->lines[i]->hit_count < ptr->hit_count) {
                        ptr = cache->lines[i];
                  }
               }
            }

            rv.hit = false;
            rv.replace = ptr->present;
            ptr->tag = fields.tag;
            ptr->hit_count = 1;
            ptr->present = true;

            copy_phy_memory_block_to_cache_line(addr, ptr, phy_memory, block_locations, sim_params.num_block_offset_bits);
            rv.value = ptr->block[fields.block_offset];
            }
            break;
        }
        default:  // Unreachable
            break;
    }
    return rv;
}
