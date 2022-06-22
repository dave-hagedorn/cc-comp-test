
#include "lib/private/support.hh"
#include "lib/static_test.hh"
#include "static_test/static_test.hh"
#include "static_test/support/support.hh"

template <size_t I>
void foo() {
    static_assert(I != 0, "invalid value");
}

STATIC_TEST("some_type", "must do something", "invalid value") {}

namespace dss = dhagedorn::static_test;

int main() {
    // dss::list_tests();

    std::cout
        << dss::_test_cases[0].to_string() << std::endl
        << dss::test_case::from_string(dss::_test_cases[0].to_string()).verb
        //.to_string()
        << std::endl;
}  // namespace dhagedorn::static_testintmain()