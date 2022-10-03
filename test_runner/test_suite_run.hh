#pragma once

#include <chrono>

#include "range/v3/all.hpp"

#include "test_case_run.hh"

namespace dhagedorn::static_tester::priv {

namespace rv = ranges::views;
namespace r = ranges;

using namespace std::chrono_literals;

struct test_suite_run {
    static_test::test_suite test_suite;
    std::vector<testcase_run> case_runs;

    auto passed() {
        return r::count_if(case_runs, [](const auto& run) {
            return run.passed();
        });
    }

    auto failed() {
        return case_runs.size() - passed();
    }

    auto duration() const {
        return r::accumulate(
            case_runs | rv::transform([](auto &r) { return r.duration; }), 0ms);
    }
};

} // namespace dhagedorn::static_tester::priv