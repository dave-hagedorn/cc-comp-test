
#include <string>
#include <type_traits>
#include <vector>

#include "comp_test/comp_test.hh"

template <typename T>
void to_string(T &&value) {
    using decayed_t = typename std::decay<T>::type;
    static_assert(std::is_integral<decayed_t>{});
    std::to_string(value);
}

// test cases can be listed standalone

// test for a static_assert with a specific message
TEST_STATIC_ASSERT("to_string", "only works on numbers", "type not supported") {
    to_string(TEST_INFO::object);
}

// test that code does not static_assert
TEST_COMPILE("to_string", "only works on numbers") {

    // test information is passed in as a TEST_INFO object
    to_string(TEST_INFO::line);
}

// alternatively, test cases can be grouped into suites

TEST_SUITE("test_types", "should all pass") {

    // arguments can be named using designated initializer synax
    TEST_STATIC_ASSERT(.thing = "to_string",
                       .will = "only works on numbers",
                       .assert_with = "non-existent assert", ) {
        to_string(TEST_INFO::object);
    }
}
