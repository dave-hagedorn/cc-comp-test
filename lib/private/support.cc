#include "static_test/support/support.hh"

#include <iostream>

/**
 * This is code that is injected by into test files
 * by static_tester
 *
 * It is written here for development to avoid needing to run
 * everything through static_tester
 *
 * These functions are for extracting information about the
 * test cases defined in a .cc file, so they can be
 * then test-compiled by static_tester
 */
namespace dhagedorn::static_test::priv {

void print_test_case_symbols() {
    for (auto& tc : _test_cases) {
        std::cout << tc.symbol << std::endl;
    }
}

}  // namespace dhagedorn::static_test::priv
