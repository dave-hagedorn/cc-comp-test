
#include <string>
#include <type_traits>
#include <vector>

#include "static_test/static_test.hh"

template <typename T>
void to_string(T &&value) {
    static_assert(
        std::is_integral<T>{} || std::is_floating_point<T>{},
        "type not supported"
    );

    std::to_string(value);
}

// STATIC_SUITE("sample 1") {

//     STATIC_TEST("to_string", "only works on numbers", "type not supported") {
//         to_string("invalid value");
//     }

//     STATIC_TEST("to_string", "only works on numbers", "type not supported") {
//         //   to_string(3);
//     }
// }

STATIC_TEST("to_string", "only works on numbers", "type not supported") {
    to_string(3);
}

STATIC_TEST("to_string", "only works on numbers", "type not supported") {
    to_string("");
}

STATIC_TEST("to_string", "only works on numbers", "type not supported") {
    to_string(TEST_INFO::value);
}

STATIC_TEST("to_string", "only works on numbers", "mismatch") {
    static_assert(TEST_INFO::object == "to_strings", "mismatch");
}