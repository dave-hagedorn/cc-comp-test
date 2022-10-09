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

// ℹ️ [1] test cases can be listed standalone

// ℹ️ [2] test for a static_assert with a specific message
MUST_STATIC_ASSERT("to_string", "only works on numbers", "type not supported") {
    to_string(TestCase::object);
}

// ℹ️ [3] test that code compiles - speicifically that it does not static_assert
MUST_COMPILE("to_string", "only works on numbers") {

    // ℹ️ [4] test information is passed in as a TestCase object
    to_string(TestCase::line);
    asdf
}

// ℹ️ [5] alternatively, test cases can be grouped into suites
TEST_SUITE("test_types", "should all pass") {

    // ℹ️ [6] arguments can be named using designated initializer synax
    MUST_STATIC_ASSERT(.object = "to_string",
                       .will = "only works on numbers",
                       .assert_with = "non-existent assert", ) {
        to_string(TestCase::object);
    }
}
