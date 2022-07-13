#pragma once

#include "static_test/comp_test.hh"

namespace dhagedorn::static_tester::priv {

struct test_suite {
    std::string name;
    std::vector<dhagedorn::static_test::test_case> test_cases;
};

} // namespace dhagedorn::static_tester::priv