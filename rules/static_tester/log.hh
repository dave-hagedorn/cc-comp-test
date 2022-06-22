#pragma once

#include "fmt/color.h"
#include "fmt/core.h"

namespace dhagedorn::static_tester::priv {

template <std::size_t N>
constexpr auto is_string_literal(const char (&)[N]) {
    return true;
}

constexpr auto is_string_literal(...) { return false; }

template <typename T>
constexpr auto is_range(T *v) -> decltype(begin(*v), end(*v), bool()) {
    return true;
}

constexpr bool is_range(...) { return false; }

template <typename T>
struct inner_type {
    using INNER = void;
};

template <template <typename...> class OUTER, typename _INNER, typename... REST>
struct inner_type<OUTER<_INNER, REST...>> {
    using INNER = _INNER;
};

template <typename T>
fmt::color color_for() {
    using DECAYED = std::decay_t<T>;
    constexpr DECAYED *dummy = 0;

    if (std::is_integral<DECAYED>{} || std::is_floating_point<DECAYED>{}) {
        return fmt::color::green;
    } else if (std::is_same<DECAYED, std::string>{} || is_string_literal(dummy)) {
        return fmt::color::yellow;
    } else if (is_range(dummy)) {
        return color_for<typename inner_type<DECAYED>::INNER>();
    }

    return fmt::color::gray;
}

void log_fields() {}

template <typename K, typename V, typename... REST>
void log_fields(K &&k, V &&v, REST &&...rest) {
    static_assert(
        sizeof...(REST) % 2 == 0,
        "rest... must contain an even "
        "number of args"
    );

    fmt::print("  {:<20} -> ", std::forward<K>(k));
    fmt::print(fmt::fg(color_for<V>()), "{}\n", std::forward<V>(v));

    log_fields(rest...);
}

template <typename... ARGS>
void log(const char *msg, ARGS &&...args) {
    fmt::print("{}\n", msg);

    log_fields(std::forward<ARGS>(args)...);
}

} // namespace dhagedorn::static_tester::priv