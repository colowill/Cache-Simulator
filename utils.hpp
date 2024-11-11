#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>

#include "types.h"

typedef struct {
    const char* memory_file;
    cache_mapping_algo cma;
} cli_args;

/**
 * Prints usage message for main.cpp and exits
 */
void print_usage_and_exit();

/**
 * @note If CLI args are invalid, prints usage message and exits
 * @param argc
 * @param argv
 * @return cli_args
 */
cli_args parse_cli(int argc, const char* argv[]);

/**
 * Pretty-prints a vector<vector<string>> table
 *
 * @param table m x n vector (all rows are assumed to be the same length)
 */
void print_table_as_grid(std::vector<std::vector<std::string>> table);

/**
 * Converts cache to a table format similar to lecture slides Cache Memory,
 * Mapping Algorithms
 *
 * @note Pass the rv to print_table_as_grid
 * @param cache
 * @param num_block_offset_bits
 * @return std::vector<std::vector<std::string>>
 */
std::vector<std::vector<std::string>> cache_to_table(
    cache* cache, unsigned int num_block_offset_bits);

/**
 * Runs the cache simulation
 *
 * @param memory_file
 * @param cma
 * @param sim_params
 */
void sim(const char* memory_file, cache_mapping_algo cma,
         simulation_parameters sim_params);

#endif  // UTILS_HPP
