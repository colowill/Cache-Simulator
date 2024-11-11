// Don't edit anything except number_of_blocks
// All other functions are correct and given to you

#include "memory.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "constants.h"

unsigned int bit_select(unsigned int num, unsigned int start_bit,
                        unsigned int end_bit) {
    start_bit++;
    num <<= (UNSIGNED_INT_NUM_BITS - start_bit);
    num >>= (UNSIGNED_INT_NUM_BITS - start_bit + end_bit);
    return num;
}

optional_uint number_of_blocks(unsigned int num_addr_bits,
                               unsigned int num_block_offset_bits) {   
    optional_uint rv = {.present = false, .value = 0};
    if (num_block_offset_bits > num_addr_bits) return rv;
  
    unsigned int num_addresses = exp2(num_addr_bits);
    unsigned int block_size = exp2(num_block_offset_bits);

    rv.value = num_addresses/block_size;
    rv.present = true;
    return rv;
}

unsigned int memory_file_to_array(const char* filepath,
                                  unsigned int* phy_memory) {
    FILE* file;
    file = fopen(filepath, "r");
    if (file == NULL) {
        fprintf(stderr, "Failed to open file %s\n", filepath);
        exit(1);
    }

    unsigned int num_addresses = 0;
    char* line = NULL;
    size_t size = 0;
    while (getline(&line, &size, file) != -1)
        phy_memory[num_addresses++] = (unsigned int)strtoul(line, NULL, 16);

    free(line);
    fclose(file);

    return ceil(log2(num_addresses));
}

unsigned int* determine_block_locations(unsigned int num_blocks,
                                        unsigned int num_block_offset_bits) {
    unsigned int* block_locations =
        (unsigned int*)malloc(sizeof(unsigned int) * num_blocks);

    for (unsigned int i = 0; i < num_blocks; i++)
        block_locations[i] = i * exp2(num_block_offset_bits);

    return block_locations;
}
