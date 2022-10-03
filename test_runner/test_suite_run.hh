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

    auto count(test_case_result with_result) const {
        auto counts = case_runs | rv::remove_if([&](auto &val) {
                          return val.result() != with_result;
                      });

        return r::distance(counts);
    }

    auto all_passed() {
        return count(test_case_result::did_static_assert) == case_runs.size();
    }

    auto duration() const {
        return r::accumulate(
            case_runs | rv::transform([](auto &r) { return r.duration; }), 0ms);
    }
};

} // namespace dhagedorn::static_tester::priv