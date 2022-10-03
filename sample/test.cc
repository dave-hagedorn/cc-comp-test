
#include <string>
#include <type_traits>
#include <vector>

#include "comp_test/comp_test.hh"

template <typename T>
void to_string(T &&value) {
    static_assert(std::is_integral<T>{} || std::is_floating_point<T>{},
                  "type not supported");

    std::to_string(value);
}

TEST_SUITE("numbers", "number cases") {

    TEST_COMP_ASSERT(
        "to_string", "only works on numbers", "type not supported") {
        to_string("invalid value");
    }

    TEST_COMP_ASSERT(
        "to_string", "only works on numbers", "type not supported") {
        to_string("");
    }
}

TEST_COMP_ASSERT("to_string", "only works on numbers", "type not supported") {
    to_string("");
}

TEST_COMP_ASSERT("to_string", "only works on numbers", "type not supported") {
    to_string(TEST_INFO::object);
}
