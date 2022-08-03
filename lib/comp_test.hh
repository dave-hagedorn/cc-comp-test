#pragma once

#include <cstdlib>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#ifdef _MSC_VER
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

#define STRINGIFY(ARG) #ARG

#define JOIN(A, B) A##B
#define EXPAND_CALL(MACRO, ...) MACRO(__VA_ARGS__)

#define UNIQUE_SYMBOL(NAME) EXPAND_CALL(JOIN, NAME, __LINE__)

template <typename T>
struct required {
    template <typename _T>
    required(_T &&v)
        : value{std::forward<_T>(v)} {}

    operator T &() { return value; }

    T value;
};

// TODO  - NAME

struct test_suite_args {
    required<std::string> name;
    required<std::string> description;
};

#define TEST_SUITE(...)                                                        \
    static auto EXPAND_CALL(JOIN, _test_suite_define, __LINE__) = [] {         \
        test_suite_args args{__VA_ARGS__};                                     \
        dhagedorn::static_test::_test_suites.push_back({                       \
            __FILE__,                                                          \
            __LINE__,                                                          \
            EXPAND_CALL(STRINGIFY, UNIQUE_SYMBOL(_test_suite_)),               \
            std::move(args.name),                                              \
            std::move(args.description),                                       \
        });                                                                    \
                                                                               \
        return 0;                                                              \
    }();                                                                       \
    namespace UNIQUE_SYMBOL(_test_suite_)

struct comp_assert_args {
    comp_assert_args(comp_assert_args &&) = delete;
    comp_assert_args(const comp_assert_args &) = delete;

    required<std::string> thing;
    required<std::string> will;
    required<std::string> assert_with;
}; // namespace comp_assert_args

#define TEST_COMP_ASSERT(...)                                                  \
    static auto EXPAND_CALL(JOIN, _static_test_define, __LINE__) = [] {        \
        comp_assert_args args{__VA_ARGS__};                                    \
        std::string pretty = std::string{__PRETTY_FUNCTION__};                 \
        dhagedorn::static_test::_test_cases.push_back({                        \
            __FILE__,                                                          \
            __LINE__,                                                          \
            __PRETTY_FUNCTION__,                                               \
            EXPAND_CALL(STRINGIFY, UNIQUE_SYMBOL(_test_case_)),                \
            std::move(args.thing),                                             \
            std::move(args.will),                                              \
            std::move(args.assert_with),                                       \
        });                                                                    \
        return 0;                                                              \
    }();                                                                       \
    template <typename TEST_INFO>                                              \
    static void UNIQUE_SYMBOL(_test_case_)()

namespace dhagedorn::static_test {

namespace detail {

class putter {
public:
    std::string str() {
        auto as_str = buf.str();
        auto len = as_str.size() > 0 ? as_str.size() - 1 : 0;

        return as_str.substr(0, len);
    }

    template <typename T>
    void operator()(T &&value) {
        buf << escape(value) << ":";
    }

private:
    std::string escape(const std::string &value) const {
        auto escaped = value;

        escaped = std::regex_replace(escaped, std::regex{":"}, R"(\:)");
        escaped = std::regex_replace(escaped, std::regex{"\n"}, R"(\n)");

        return escaped;
    }

    template <typename T>
    const T &escape(const T &value) const {
        return value;
    }

    std::stringstream buf;
};

struct getter {
public:
    getter(const std::string &value)
        : _value{value} {}

    std::string operator()() {
        std::stringstream token;
        char c;

        while (take(c)) {
            if (c == ':' || c == '\n') {
                break;
            }

            if (c != '\\') {
                token << c;
                continue;
            }

            if (!take(c)) {
                continue;
            }

            if (c == 'n') {
                token << '\n';
                continue;
            }

            token << c;
        }

        return token.str();
    };

private:
    unsigned _i = 0;
    std::string _value;

    bool take(char &out) {
        if (_i >= _value.size()) {
            return false;
        }

        out = _value[_i++];
        return true;
    };
};

} // namespace detail

struct test_case {
    std::string file;
    unsigned long line;

    // "namespace"
    std::string detailed_name;
    std::string symbol;
    std::string object;
    std::string verb;
    std::string expected_assert_message;

    // clang-format off
    // clang: "auto test::(anonymous class)::operator()() const"
    // gcc: "test::<lambda()>\000"
    // msvc: "auto __cdecl test::<lambda_676ec28c60ffff024507b007ccd4a443>::operator()(void) const"
    // For no NS = remove "test::"
    // clang-format on
    std::string test_suite_symbol() const {
        std::regex reg{R"(.*\s+([\w\d_]+)::(\(anonymous|<lambda).*)"};

        std::smatch match;
        if (std::regex_search(detailed_name, match, reg)) {
            return match[1].str();
        }

        return "";
    }

    std::string to_string() const {
        detail::putter put;

        put(file);
        put(line);
        put(detailed_name);
        put(symbol);
        put(object);
        put(verb);
        put(expected_assert_message);

        return put.str();
    }

    static test_case from_string(const std::string &value) {
        detail::getter get{value};

        try {
            return test_case{
                get(),
                std::stoul(get()),
                get(),
                get(),
                get(),
                get(),
                get(),
            };
        } catch (std::exception &exception) {
            std::cerr << exception.what() << std::endl;
            throw exception;
        }
    }
};

struct test_suite {
    std::string file;
    unsigned long line;

    std::string symbol;
    std::string name;
    std::string description;

    std::string to_string() const {
        detail::putter put;

        put(file);
        put(line);
        put(symbol);
        put(name);
        put(description);

        return put.str();
    }

    static test_suite from_string(const std::string &value) {
        detail::getter get{value};

        try {
            return test_suite{
                get(),
                std::stoul(get()),
                get(),
                get(),
                get(),
            };
        } catch (std::exception &exception) {
            std::cerr << exception.what() << std::endl;
            throw exception;
        }
    }

    // filled in later
    std::vector<test_case> cases;
};

inline std::vector<test_case> _test_cases;
inline std::vector<test_suite> _test_suites;

} // namespace dhagedorn::static_test