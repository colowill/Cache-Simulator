// For usage, see utils.cpp#print_usage_and_exit

#include "constants.h"
#include "utils.hpp"

int _main(int argc, const char* argv[]) {
    cli_args args = parse_cli(argc, argv);
    sim(args.memory_file, args.cma, SIMULATION_PARAMETERS);

    return EXIT_SUCCESS;
}

/**
 * tests.cpp needs to test the main function
 * However, GTest tests executable already defines a main function so can't
 * include main function
 * In Makefile, for tests executable, this file compiled with flag -DTEST_MODE
 * to exclude main function
 */
#ifndef TEST_MODE

int main(int argc, const char* argv[]) { return _main(argc, argv); }

#endif  // TEST_MODE
