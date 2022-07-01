#pragma once

#include <chrono>
#include <unordered_map>

#include "fmt/chrono.h"
#include "fmt/core.h"
#include "range/v3/all.hpp"
#include "tinyxml2.h"

#include "compiler.hh"
#include "static_test/comp_test.hh"
#include "util.hh"

namespace dhagedorn::static_tester::priv {

using namespace std::string_literals;

enum class test_case_result {
    skipped, // TODO
    did_static_assert,
    other_compile_failure,
    compiled,
};

struct testsuite_run {};

struct testcase_run {
    static_test::test_case tc;
    std::optional<compile_result> compiler_output;
    std::chrono::milliseconds duration;

    auto result() const {
        if (!compiler_output) {
            return test_case_result::skipped; // TODO
        }

        auto compiled = compiler_output->compiled;
        auto did_assert
            = ranges::any_of(compiler_output->diagnostics, [this](auto &diag) {
                  auto is_error = diag.sev == severity::error;
                  auto is_static_assert
                      = diag.message.find("static_assert") != std::string::npos;

                  auto assert_matches
                      = diag.message.find(tc.expected_assert_message)
                        != std::string::npos;

                  return is_error && is_static_assert && assert_matches;
              });

        if (compiled) {
            return test_case_result::compiled;
        }

        if (did_assert) {
            return test_case_result::did_static_assert;
        }

        return test_case_result::other_compile_failure;
    }
};

} // namespace dhagedorn::static_tester::priv