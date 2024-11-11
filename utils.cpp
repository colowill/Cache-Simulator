#include "utils.hpp"

#include <math.h>

#include <cstring>
#include <exception>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "cache.h"
#include "constants.h"
#include "memory.h"
#include "types.h"

namespace fs = std::filesystem;

void print_usage_and_exit() {
    std::cerr << "Usage: ./main memory_file dm|fa\n\n";
    std::cerr << "memory_file: text file that represents "
                 "values in physical memory, one hex value per line\n";
    std::cerr
        << "dm|fa: select one of two cache "
           "mapping algorithms, either direct mapping or fully associative\n\n";
    std::cerr << "Example: ./main data/memory.txt dm\n";
    exit(1);
}

cli_args parse_cli(int argc, const char* argv[]) {
    if (argc < 3) {
        std::cerr << "Incorrect number of arguments: " << argc << "\n\n";
        print_usage_and_exit();
    }

    if (!fs::exists(argv[1])) {
        std::cerr << "memory_file " << argv[1]
                  << " is not a filepath or does not exist\n";
        print_usage_and_exit();
    }

    if (strcmp("dm", argv[2]) != 0 && strcmp("fa", argv[2]) != 0) {
        std::cerr
            << argv[2]
            << " is not a valid cache mapping function, must be dm or fa\n";
        print_usage_and_exit();
    }

    cli_args rv = {.memory_file = argv[1],
                   .cma = strcmp("dm", argv[2]) == 0 ? DM : FA};
    return rv;
}

void print_table_as_grid(std::vector<std::vector<std::string>> table) {
    // Width of each column is the length of longest element in each column
    std::vector<std::size_t> column_widths;
    for (std::size_t col = 0; col < table[0].size(); col++) {
        std::size_t max_width = 0;
        for (const auto& row : table)
            max_width = std::max(max_width, row[col].size());
        column_widths.push_back(max_width);
    }

    // Helper function that prints a horizontal line of chars
    auto print_line = [&](const char left, const char middle, const char right,
                          const char fill) {
        std::cout << left;
        for (size_t col = 0; col < column_widths.size(); ++col) {
            std::cout << std::string(column_widths[col] + 2,
                                     fill);  // Add padding
            if (col < column_widths.size() - 1) std::cout << middle;
        }
        std::cout << right << "\n";
    };

    // Print top border
    print_line('+', '+', '+', '-');

    // Print each row
    for (long unsigned int row = 0; row < table.size(); row++) {
        // Print row content
        for (size_t col = 0; col < table[row].size(); col++)
            std::cout << "| " << std::left << std::setw(column_widths[col])
                      << table[row][col] << " ";
        std::cout << "|\n";

        if (row == table.size() - 1)
            print_line('+', '+', '+', '-');
        else
            print_line('+', '+', '+', '-');
    }
}

std::vector<std::vector<std::string>> cache_to_table(
    cache* cache, unsigned int num_block_offset_bits) {
    std::vector<std::vector<std::string>> rv;

    unsigned int block_size = exp2(num_block_offset_bits);

    std::vector<std::string> header1{"", "", "", "", "b"};
    std::vector<std::string> header2{"line", "present", "tag", "#hits"};

    for (unsigned int i = 0; i < block_size - 1; i++) header1.push_back("");
    for (unsigned int i = 0; i < block_size; i++)
        header2.push_back(std::to_string(i));

    rv.push_back(header1);
    rv.push_back(header2);

    cache_line** lines = cache->lines;
    // TODO: This should be cleaned up, just make an outer if (line->present)
    // and set row based on that
    for (unsigned int i = 0; i < cache->num_lines; i++) {
        cache_line* line = lines[i];
        std::stringstream tag_hex;
        tag_hex << std::hex << "0x" << line->tag;

        std::vector<std::string> row{
            std::to_string(i), line->present ? "y" : "n",
            line->present ? tag_hex.str() : "",
            line->present ? std::to_string(line->hit_count) : ""};
        for (unsigned int i = 0; i < block_size; i++) {
            std::stringstream block_val_hex;
            block_val_hex << std::hex << "0x" << line->block[i];
            row.push_back(line->present ? block_val_hex.str() : "");
        }
        rv.push_back(row);
    }

    return rv;
}

void sim(const char* memory_file, cache_mapping_algo cma,
         simulation_parameters sim_params) {
    unsigned int* phy_memory =
        (unsigned int*)malloc(sizeof(unsigned int) * MAX_PHY_MEMORY_LENGTH);
    unsigned int computed_num_addr_bits =
        memory_file_to_array(memory_file, phy_memory);
    if (sim_params.num_addr_bits != computed_num_addr_bits) {
        std::cerr << "num_addr_bits for " << memory_file
                  << " was computed to be: " << computed_num_addr_bits
                  << ", but this simulator currently only supports the "
                     "hardcoded sim_params.num_addr_bits value: "
                  << sim_params.num_addr_bits << "\n";
        exit(1);
    }
    optional_uint optional_num_blocks = number_of_blocks(
        sim_params.num_addr_bits, sim_params.num_block_offset_bits);
    if (!optional_num_blocks.present) {
        std::cerr << "num_addr_bits is " << sim_params.num_addr_bits << "\n";
        std::cerr << "num_block_offset_bits is "
                  << sim_params.num_block_offset_bits << "\n";
        std::cerr << "However, these inputs are invalid, so number_of_blocks "
                     "cannot compute a valid number "
                     "of blocks.\n";
        exit(1);
    }
    unsigned int num_blocks = optional_num_blocks.value;

    std::cout << std::dec << "Physical memory address number of bits: "
              << sim_params.num_addr_bits << "\n";
    std::cout << std::dec << "Number of blocks: " << num_blocks << "\n\n";

    unsigned int* block_locations =
        determine_block_locations(num_blocks, sim_params.num_block_offset_bits);
    cache* cache = initialize_cache(sim_params.num_line_bits,
                                    sim_params.num_block_offset_bits);

    std::string input;
    unsigned int addr;
    while (true) {
        std::cout
            << "Enter " << sim_params.num_addr_bits
            << "-bit memory address in hex format or \"exit\" to exit: 0x";
        std::cin >> input;
        if (input == "exit") break;

        try {
            addr = stoul(input, nullptr, 16);
        } catch (const std::invalid_argument& e) {
            std::cerr
                << "Error: invalid input (not a valid hex number). Try again\n";
        } catch (std::out_of_range& e) {
            std::cerr << "Error: out of range of unsigned long. Try again\n";
        } catch (const std::exception& e) {
            std::cerr << "Exception caught: " << e.what() << ". Try again\n";
        } catch (...) {
            std::cerr << "Unknown exception occurred. Try again\n";
        }
        if (addr >= 1u << sim_params.num_addr_bits) {
            std::cerr << std::hex
                      << "Error: address is out of range of physical memory "
                         "num_addr_bits: "
                      << sim_params.num_addr_bits << " (max value is 0x"
                      << ((1u << sim_params.num_addr_bits) - 1)
                      << "). Try again\n";
            continue;
        }

        cache_read_rv rv = cache_read(cache, addr, cma, sim_params,
                                      block_locations, phy_memory);
        std::cout << "\n";
        print_table_as_grid(
            cache_to_table(cache, sim_params.num_block_offset_bits));

        if (rv.present) {
            if (rv.hit) {
                std::cout << std::hex << "\nCache hit, retrieved value 0x"
                          << rv.value << ", which is also at address 0x" << addr
                          << "\n";
            } else {
                std::cout << std::hex << "\nCache miss, value 0x" << rv.value
                          << " from address 0x" << addr
                          << " was copied into cache (along with the rest of "
                             "the block)\n";
                if (rv.replace)
                    std::cout
                        << "Replacement, an in-use cache line was evicted\n\n";
                else
                    std::cout << "No replacement, a cache line that was not "
                                 "in-use is now in-use\n\n";
            }
            if (rv.value != phy_memory[addr])
                std::cout
                    << "Error: cached value for address != phy_memory[addr]\n";
        } else {
            std::cerr << "\n"
                      << std::hex << "cache_read failed for given address 0x"
                      << addr << "\n";
        }
    }
}
