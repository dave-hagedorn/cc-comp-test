
#include <string>
#include <type_traits>
#include <vector>

#include "comp_test/comp_test.hh"

template <typename T>
void to_string(T &&value) {
    using decayed_t = typename std::decay<T>::type;

    static_assert(std::is_integral<decayed_t>{}
                      || std::is_floating_point<decayed_t>{},
                  "type not supported");

    std::to_string(value);
}

// Grouping tests by suite
TEST_SUITE("test_types", "should all pass") {
    TEST_COMP_ASSERT(
        "to_string", "only works on numbers", "type not supported") {
        to_string(TEST_INFO::object);
    }

    TEST_COMPILES("to_string", "only works on numbers", "should compile") {
        to_string(TEST_INFO::line);
    }
}

TEST_SUITE("test_types", "should all fail") {
    // should pass in test results
    TEST_COMP_ASSERT(
        "to_string", "only works on numbers", "non-existent assert") {
        to_string(TEST_INFO::object);
    }

    TEST_COMPILES("to_string", "only works on numbers", "should compile") {
        to_string(TEST_INFO::object);
    }
}

TEST_SUITE("test_types", "should all error") {
    // should pass in test results
    TEST_COMP_ASSERT(
        "to_string", "only works on numbers", "type not supported") {
        to_str(TEST_INFO::OBJECT);
    }

    TEST_COMPILES("to_string", "only works on numbers", "should compile") {
        to_str(TEST_INFO::OBJECT);
    }
}
