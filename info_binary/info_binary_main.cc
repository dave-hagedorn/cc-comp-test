
#include "comp_test/comp_test.hh"

namespace dst = dhagedorn::static_test;

int main(int argc, char **argv) {
    std::cout << "info binary - lists test cases and suites\n";

    for (const auto &ts : dst::_test_suites()) {
        std::cout << "test_suite:" << ts.to_string() << std::endl;
    }

    for (const auto &tc : dst::_test_cases()) {
        std::cout << "test_case:" << tc.to_string() << std::endl;
    }

    return 0;
}