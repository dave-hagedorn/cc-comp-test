#pragma once

#include <chrono>
#include <unordered_map>

#include "fmt/chrono.h"
#include "fmt/core.h"
#include "range/v3/all.hpp"
#include "tinyxml2.h"

#include "comp_test/comp_test.hh"
#include "compiler.hh"
#include "util.hh"

namespace dhagedorn::static_tester::priv {

using namespace std::string_literals;

enum class test_case_action {
    skipped, // TODO
    did_static_assert,
    other_compile_failure,
    compiled,
};

enum class test_case_result {
    pass,
    fail,
    error,
    skipped,
};

struct testcase_run {
    static_test::test_case tc;
    std::optional<compile_result> compiler_output;
    std::chrono::milliseconds duration;

    auto result() const {
        if (!compiler_output) {
            return test_case_result::skipped; // TODO
        }

        switch (tc.type) {
            case static_test::test_type::MUST_COMPILE:
                return when(compiler_output->compiled,
                            test_case_result::pass,
                            compiler_output->did_static_assert(),
                            test_case_result::fail,
                            test_case_result::error);
            case static_test::test_type::MUST_STATIC_ASSERT:
                return when(compiler_output->has_static_assert(
                                tc.expected_assert_message),
                            test_case_result::pass,
                            compiler_output->did_static_assert(),
                            test_case_result::fail,
                            compiler_output->compiled,
                            test_case_result::fail,
                            test_case_result::error);
        }
    }

    std::optional<std::string> fail_or_error_message() const {
        switch (tc.type) {
            case static_test::test_type::MUST_COMPILE:
                return when<std ::string>(
                    compiler_output->compiled,
                    {},
                    compiler_output->did_static_assert(),
                    fmt::format(
                        R"(case should have compiled, but asserted with "{}")",
                        *compiler_output->static_assert_msg()),
                    "case should have compiled, but failed to - see "
                    "stdout/stderr");
            case static_test::test_type::MUST_STATIC_ASSERT:
                return when<std ::string>(
                    compiler_output->compiled,
                    fmt::format(
                        R"(case must static_assert with "{}", but compiled)",
                        tc.expected_assert_message),
                    compiler_output->did_static_assert(),
                    fmt::format(
                        R"(case must static_assert with "{}", but asserted with "{}")",
                        tc.expected_assert_message,
                        *compiler_output->static_assert_msg()),
                    fmt::format(
                        R"(case must static_assert with "{}", but failed to compile and raised no static_assert)",
                        tc.expected_assert_message));
            default:
                return "";
        }
    };
};

} // namespace dhagedorn::static_tester::priv