
#include "static_test/static_test.hh"

namespace dst = dhagedorn::static_test;

int main(int argc, char **argv) {
    for (const auto &tc : dst::_test_cases) {
        std::cout << tc.to_string() << std::endl;
    }

    return 0;
}