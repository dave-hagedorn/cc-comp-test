#pragma once

#include <chrono>

#include "range/v3/all.hpp"

#include "test_case_run.hh"

namespace dhagedorn::comp_test::impl {

namespace rv = ranges::views;
namespace r = ranges;

using namespace std::chrono_literals;

struct test_suite_run {
    comp_test::test_suite test_suite;
    std::vector<testcase_run> case_runs;

    auto passed() const {
        return r::count_if(case_runs, [](const auto &run) {
            return run.result() == test_case_result::pass;
        });
    }

    auto failed() const {
        return r::count_if(case_runs, [](const auto &run) {
            return run.result() == test_case_result::fail;
        });
    }

    auto errors() const {
        return r::count_if(case_runs, [](const auto &run) {
            return run.result() == test_case_result::error;
        });
    }

    auto duration() const {
        return r::accumulate(
            case_runs | rv::transform([](auto &r) { return r.duration; }), 0ms);
    }
};

} // namespace dhagedorn::comp_test::impl