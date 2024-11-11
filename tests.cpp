#include "tests.hpp"

#include <cstring>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "cache.h"
#include "constants.h"
#include "main.cpp"
#include "memory.h"
#include "types.h"

const unsigned int ADDR_8BIT_1 = 0b10010110;
const unsigned int ADDR_8BIT_2 = 0b11111000;
const unsigned int ADDR_8BIT_3 = 0b10010011;

const unsigned int ADDR_16BIT_1 = 0x1234;
const unsigned int ADDR_16BIT_2 = 0xdead;

const simulation_parameters SIM_PARAMS_ALT = {
    .num_addr_bits = 16, .num_line_bits = 8, .num_block_offset_bits = 4};

const char* MEMORY_FILE = "data/memory.txt";
// 16 address bits means 65536 values (each still a byte)
const char* MEMORY_FILE_ALT = "data/memory65536.txt";

const unsigned long BUFSIZE_FOR_CAPTURING_STDOUT = 1 << 16;

const int CACHE_READ_POINTS_PER_TEST_CASE = 5;

SAFE_TEST(NumberOfBlocks, DefaultSimParams, {
    optional_uint rv =
        number_of_blocks(SIMULATION_PARAMETERS.num_addr_bits,
                         SIMULATION_PARAMETERS.num_block_offset_bits);
    EXPECT_TRUE(rv.present);
    EXPECT_EQ(64, rv.value);
})

SAFE_TEST(NumberOfBlocks, ValidAlternate, {
    optional_uint rv = number_of_blocks(SIM_PARAMS_ALT.num_addr_bits,
                                        SIM_PARAMS_ALT.num_block_offset_bits);
    EXPECT_TRUE(rv.present);
    EXPECT_EQ(4096, rv.value);
})

SAFE_TEST(NumberOfBlocks, Invalid_0_2,
          { EXPECT_FALSE(number_of_blocks(0, 2).present); })

SAFE_TEST(NumberOfBlocks, Invalid_4_8,
          { EXPECT_FALSE(number_of_blocks(4, 8).present); })

SAFE_TEST(ExtractFields, DMDefaultSimParams1, {
    addr_fields rv = extract_fields(ADDR_8BIT_1, DM, SIMULATION_PARAMETERS);

    EXPECT_EQ(0b100, rv.tag);
    EXPECT_TRUE(rv.line.present);
    EXPECT_EQ(0b101, rv.line.value);
    EXPECT_EQ(0b10, rv.block_offset);
})

SAFE_TEST(ExtractFields, DMDefaultSimParams2, {
    addr_fields rv = extract_fields(ADDR_8BIT_2, DM, SIMULATION_PARAMETERS);

    EXPECT_EQ(0b111, rv.tag);
    EXPECT_TRUE(rv.line.present);
    EXPECT_EQ(0b110, rv.line.value);
    EXPECT_EQ(0b00, rv.block_offset);
})

SAFE_TEST(ExtractFields, DMAltSimParams, {
    addr_fields rv = extract_fields(ADDR_16BIT_1, DM, SIM_PARAMS_ALT);

    EXPECT_EQ(0x1, rv.tag);
    EXPECT_TRUE(rv.line.present);
    EXPECT_EQ(0x23, rv.line.value);
    EXPECT_EQ(0x4, rv.block_offset);
})

SAFE_TEST(ExtractFields, FADefaultSimParams1, {
    addr_fields rv = extract_fields(ADDR_8BIT_1, FA, SIMULATION_PARAMETERS);

    EXPECT_EQ(0b100101, rv.tag);
    EXPECT_FALSE(rv.line.present);
    EXPECT_EQ(0b10, rv.block_offset);
})

SAFE_TEST(ExtractFields, FADefaultSimParams2, {
    addr_fields rv = extract_fields(ADDR_8BIT_2, FA, SIMULATION_PARAMETERS);

    EXPECT_EQ(0b111110, rv.tag);
    EXPECT_FALSE(rv.line.present);
    EXPECT_EQ(0b00, rv.block_offset);
})

SAFE_TEST(ExtractFields, FAAltSimParams, {
    addr_fields rv = extract_fields(ADDR_16BIT_1, FA, SIM_PARAMS_ALT);

    EXPECT_EQ(0x123, rv.tag);
    EXPECT_FALSE(rv.line.present);
    EXPECT_EQ(0x4, rv.block_offset);
})

class CommonTestData : public ::testing::Test {
   protected:
    unsigned int* phy_memory;
    simulation_parameters sim_params;
    unsigned int num_blocks;
    unsigned int* block_locations;
    cache* _cache;

    unsigned int* phy_memory_alt;
    simulation_parameters sim_params_alt;
    unsigned int num_blocks_alt;
    unsigned int* block_locations_alt;
    cache* cache_alt;

    void SetUp() override {
        phy_memory =
            (unsigned int*)malloc(sizeof(unsigned int) * MAX_PHY_MEMORY_LENGTH);
        memory_file_to_array(MEMORY_FILE, phy_memory);
        sim_params = SIMULATION_PARAMETERS;
        num_blocks = number_of_blocks(sim_params.num_addr_bits,
                                      sim_params.num_block_offset_bits)
                         .value;
        block_locations = determine_block_locations(
            num_blocks, sim_params.num_block_offset_bits);
        _cache = initialize_cache(sim_params.num_line_bits,
                                  sim_params.num_block_offset_bits);

        phy_memory_alt =
            (unsigned int*)malloc(sizeof(unsigned int) * MAX_PHY_MEMORY_LENGTH);
        memory_file_to_array(MEMORY_FILE_ALT, phy_memory_alt);
        sim_params_alt = SIM_PARAMS_ALT;
        num_blocks_alt = number_of_blocks(sim_params_alt.num_addr_bits,
                                          sim_params_alt.num_block_offset_bits)
                             .value;
        block_locations_alt = determine_block_locations(
            num_blocks_alt, sim_params_alt.num_block_offset_bits);
        cache_alt = initialize_cache(sim_params_alt.num_line_bits,
                                     sim_params_alt.num_block_offset_bits);
    }

    void TearDown() override {}
};

SAFE_TEST_F(CommonTestData, CopyPhyMemoryBlockToCacheLine_DefaultSimParams, {
    // Hardcoding here to avoid making the code obvious
    unsigned int block_size = 4;

    unsigned int addr = 0b101;
    unsigned int block_start_addr = 4;
    // The line used is arbitrary since the function should copy to the given
    // line regardless of cma
    cache_line* line = _cache->lines[1];
    copy_phy_memory_block_to_cache_line(addr, line, phy_memory, block_locations,
                                        sim_params.num_block_offset_bits);
    for (unsigned int i = 0; i < block_size; i++)
        EXPECT_EQ(phy_memory[block_start_addr + i], line->block[i]);

    addr = ADDR_8BIT_1;
    block_start_addr = 148;
    line = _cache->lines[0b101];
    copy_phy_memory_block_to_cache_line(addr, line, phy_memory, block_locations,
                                        sim_params.num_block_offset_bits);
    for (unsigned int i = 0; i < block_size; i++)
        EXPECT_EQ(phy_memory[block_start_addr + i], line->block[i]);

    addr = ADDR_8BIT_2;
    block_start_addr = 248;
    line = _cache->lines[0b110];
    copy_phy_memory_block_to_cache_line(addr, line, phy_memory, block_locations,
                                        sim_params.num_block_offset_bits);
    for (unsigned int i = 0; i < block_size; i++)
        EXPECT_EQ(phy_memory[block_start_addr + i], line->block[i]);

    addr = ADDR_8BIT_3;
    block_start_addr = 144;
    line = _cache->lines[0b100];
    copy_phy_memory_block_to_cache_line(addr, line, phy_memory, block_locations,
                                        sim_params.num_block_offset_bits);
    for (unsigned int i = 0; i < block_size; i++)
        EXPECT_EQ(phy_memory[block_start_addr + i], line->block[i]);
})

SAFE_TEST_F(CommonTestData, CopyPhyMemoryBlockToCacheLine_AltSimParams, {
    // Hardcoding here to avoid making the code obvious
    unsigned int block_size = 16;

    unsigned int addr = ADDR_16BIT_1;
    unsigned int block_start_addr = 4656;
    cache_line* line = cache_alt->lines[0x23];
    copy_phy_memory_block_to_cache_line(addr, line, phy_memory_alt,
                                        block_locations_alt,
                                        sim_params_alt.num_block_offset_bits);
    for (unsigned int i = 0; i < block_size; i++)
        EXPECT_EQ(phy_memory_alt[block_start_addr + i], line->block[i]);

    addr = ADDR_16BIT_2;
    block_start_addr = 56992;
    line = cache_alt->lines[0xea];
    copy_phy_memory_block_to_cache_line(addr, line, phy_memory_alt,
                                        block_locations_alt,
                                        sim_params_alt.num_block_offset_bits);
    for (unsigned int i = 0; i < block_size; i++)
        EXPECT_EQ(phy_memory_alt[block_start_addr + i], line->block[i]);
})

/**
 * This test case checks that the output of ./main data/memory.txt dm <
 * data/dm_in*.txt is the same as data/dm_out*.txt
 *
 * Because cache_read is the final function to be implemented,
 * if all other functions are correct (i.e., all other tests pass),
 * AND cache_read is correct, the main function will have correct output
 *
 * That is, assuming all other test cases pass, this test case
 * passes if cache_read (DM mode) is correct and fails if cache_read (DM mode)
 * is incorrect
 *
 * To debug your cache_read function, run the main
 * executable as specified in the README section Running the main executable.
 * Given an input file, redirect your output to a temp file and compare it with
 * the expected output (use diff). Any differences would imply that your
 * cache_read is incorrect
 *
 * @note Don't attempt cache_read until all other test cases pass
 */
TEST(CacheReadDM,
     RunMainFuncOnDMInputFilesInDataDirAndCompareOutputToExpected) {
    // Doesn't have to be mmapped because the child doesn't write to this, only
    // reads
    std::vector<std::string> input_files;
    // Must be mmapped because the child writes to this variable
    // Without mmap, the child would operate on its copy of this, so the parent
    // wouldn't see the modifcation
    int* shared_pass = (int*)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
                                  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    for (const fs::directory_entry& dir_entry :
         fs::recursive_directory_iterator(DATA_DIR))
        if (dir_entry.is_regular_file() &&
            dir_entry.path().string().find("dm_in") != std::string::npos)
            input_files.push_back(dir_entry.path().string());
    RecordProperty("total",
                   size(input_files) * CACHE_READ_POINTS_PER_TEST_CASE);
    run_with_signal_catching([&input_files, &shared_pass]() {
        const int argc = 3;
        for (auto& input_file : input_files) {
            const char* argv[] = {"./main", MEMORY_FILE, "dm", NULL};

            // Redirect stdin from input file
            int input_fd = open(input_file.c_str(), O_RDONLY);
            int original_stdin_fd = dup(STDIN_FILENO);
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);

            // Since _main prints to stdout, capture stdout
            int pipe_fd[2];
            pipe(pipe_fd);
            int original_stdout_fd = dup(STDOUT_FILENO);
            dup2(pipe_fd[1], STDOUT_FILENO);
            close(pipe_fd[1]);

            _main(argc, argv);

            fflush(stdout);
            dup2(original_stdout_fd, STDOUT_FILENO);
            close(original_stdout_fd);

            char buffer[BUFSIZE_FOR_CAPTURING_STDOUT];
            ssize_t bytes_read = read(pipe_fd[0], buffer, sizeof(buffer) - 1);
            buffer[bytes_read] = '\0';
            close(pipe_fd[0]);

            std::string captured_stdout = buffer;
            std::string expected_output_path = input_file;
            size_t pos = expected_output_path.find("in");
            if (pos != std::string::npos)
                expected_output_path.replace(pos, 2, "out");
            std::ifstream expected_output_file(expected_output_path);
            std::string expected(
                (std::istreambuf_iterator<char>(expected_output_file)),
                std::istreambuf_iterator<char>());
            std::string actual = captured_stdout;
            if (expected.compare(actual) == 0)
                *shared_pass += CACHE_READ_POINTS_PER_TEST_CASE;

            EXPECT_STREQ(expected.c_str(), actual.c_str())
                << "after running _main with argc " << argc << ", argv {\""
                << argv[0] << "\", \"" << argv[1] << "\", \"" << argv[2]
                << "\"}, and stdin from " << input_file;

            // Restore stdin
            dup2(original_stdin_fd, STDIN_FILENO);
            close(original_stdin_fd);
        }
    });
    RecordProperty("pass", *shared_pass);
    munmap(shared_pass, sizeof(int));
}

// Same as above but for FA
// Unfortunately, can't remove code duplication by extracting to a helper
// function
TEST(CacheReadFA,
     RunMainFuncOnFAInputFilesInDataDirAndCompareOutputToExpected) {
    // Doesn't have to be mmapped because the child doesn't write to this, only
    // reads
    std::vector<std::string> input_files;
    // Must be mmapped because the child writes to this variable
    // Without mmap, the child would operate on its copy of this, so the parent
    // wouldn't see the modifcation
    int* shared_pass = (int*)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
                                  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    for (const fs::directory_entry& dir_entry :
         fs::recursive_directory_iterator(DATA_DIR))
        if (dir_entry.is_regular_file() &&
            dir_entry.path().string().find("fa_in") != std::string::npos)
            input_files.push_back(dir_entry.path().string());
    RecordProperty("total",
                   size(input_files) * CACHE_READ_POINTS_PER_TEST_CASE);
    run_with_signal_catching([&input_files, &shared_pass]() {
        const int argc = 3;
        for (auto& input_file : input_files) {
            const char* argv[] = {"./main", MEMORY_FILE, "fa", NULL};

            // Redirect stdin from input file
            int input_fd = open(input_file.c_str(), O_RDONLY);
            int original_stdin_fd = dup(STDIN_FILENO);
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);

            // Since _main prints to stdout, capture stdout
            int pipe_fd[2];
            pipe(pipe_fd);
            int original_stdout_fd = dup(STDOUT_FILENO);
            dup2(pipe_fd[1], STDOUT_FILENO);
            close(pipe_fd[1]);

            _main(argc, argv);

            fflush(stdout);
            dup2(original_stdout_fd, STDOUT_FILENO);
            close(original_stdout_fd);

            char buffer[BUFSIZE_FOR_CAPTURING_STDOUT];
            ssize_t bytes_read = read(pipe_fd[0], buffer, sizeof(buffer) - 1);
            buffer[bytes_read] = '\0';
            close(pipe_fd[0]);

            std::string captured_stdout = buffer;
            std::string expected_output_path = input_file;
            size_t pos = expected_output_path.find("in");
            if (pos != std::string::npos)
                expected_output_path.replace(pos, 2, "out");
            std::ifstream expected_output_file(expected_output_path);
            std::string expected(
                (std::istreambuf_iterator<char>(expected_output_file)),
                std::istreambuf_iterator<char>());
            std::string actual = captured_stdout;
            if (expected.compare(actual) == 0)
                *shared_pass += CACHE_READ_POINTS_PER_TEST_CASE;

            EXPECT_STREQ(expected.c_str(), actual.c_str())
                << "after running _main with argc " << argc << ", argv {\""
                << argv[0] << "\", \"" << argv[1] << "\", \"" << argv[2]
                << "\"}, and stdin from " << input_file;

            // Restore stdin
            dup2(original_stdin_fd, STDIN_FILENO);
            close(original_stdin_fd);
        }
    });
    RecordProperty("pass", *shared_pass);
    munmap(shared_pass, sizeof(int));
}
