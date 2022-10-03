#pragma once

#include <initializer_list>
#include <optional>

#include "range/v3/all.hpp"

namespace dhagedorn::static_tester::priv {

template <typename D>
inline auto join(D &&d) {
    return ranges::views::join(std::forward<D>(d)) | ranges::to<std::string>();
}

struct then_path {
    const bool take;

    template <typename F>
    auto then(F &&f) -> std::optional<decltype(f())> {
        if (take) {
            return f();
        }

        return {};
    }
};

inline auto opt_if = [](const bool test) {
    return then_path{
        test,
    };
};

template <typename T, typename U>
inline bool operator&&(T &&t, std::initializer_list<U> u) {
    for (auto c : u) {
        if (c == t) {
            return true;
        }
    }

    return false;
}

template <typename FCT>
struct defer {
    defer(FCT &&f)
        : _f{std::move(f)} {}
    ~defer() { _f(); }

private:
    FCT _f;
};

} // namespace dhagedorn::static_tester::priv