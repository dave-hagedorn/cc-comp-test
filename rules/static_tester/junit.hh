#pragma once

#include <chrono>
#include <cstdio>
#include <ostream>
#include <string>

#include "boost/filesystem.hpp"
#include "fmt/core.h"
#include "log.hh"
#include "range/v3/all.hpp"
#include "rules/static_tester/test_suite_run.hh"
#include "tinyxml2.h"

#include "test_case_run.hh"
#include "util.hh"

namespace dhagedorn::static_tester::priv {

#include "boost/filesystem.hpp"

using namespace std::chrono_literals;

namespace bfs = boost::filesystem;

/* Bazel:
<?xml version="1.0" encoding="UTF-8"?>
<testsuites>
  <testsuite name="sample/test.sh" tests="1" failures="0" errors="0">
    <testcase name="sample/test.sh" status="run" duration="1"
time="1"></testcase>
*/

class junit {
public:
    auto write(const std::vector<test_suite_run> &runs, bfs::path path) {

        FILE *fout = fopen(path.c_str(), "wb");

        if (!fout) {
            // TODO XML_OUTPUT_FILE relative to workspace root, but we're not
            // running from there!
            std::cerr << path << strerror(errno) << std::endl;
            // throw "could not open";
        }

        auto cleanup = defer{[&] {
            if (fout) {
                fclose(fout);
            }
        }};

        tinyxml2::XMLPrinter p{fout};

        p.PushHeader(false, true);

        p.OpenElement("testsuites");

        for (const auto &suite : runs) {
            _start_suite(suite, p);
        }

        p.CloseElement();

        fflush(fout);
    }

private:
    template <typename T> auto _sec(T &&val) {
        return 1.0f
               * std::chrono::duration_cast<std::chrono::milliseconds>(val)
                     .count()
               / 1000;
    };

    void _start_suite(const test_suite_run &run, tinyxml2::XMLPrinter &p) {
        p.OpenElement("testsuite");

        p.PushAttribute("name", run.test_suite.name.c_str());
        p.PushAttribute("tests", run.test_suite.test_cases.size());
        p.PushAttribute("failures", run.count(test_case_result::compiled));
        p.PushAttribute("errors",
                        run.count(test_case_result::other_compile_failure));
        p.PushAttribute("time", _sec(run.duration()));

        for (auto &tc : run.case_runs) {
            _add_tc(tc, p);
        }

        p.CloseElement();
    }

    void _add_tc(const testcase_run &run, tinyxml2::XMLPrinter &p) {
        p.OpenElement("testcase");
        p.PushAttribute("classname", run.tc.object.c_str());
        p.PushAttribute("name", run.tc.verb.c_str());
        p.PushAttribute("status",
                        run.result() == test_case_result::skipped ? "notrun"
                                                                  : "run");

        p.PushAttribute("duration", _sec(run.duration));
        p.PushAttribute("time", _sec(run.duration));

        if (run.result() == test_case_result::other_compile_failure) {
            p.OpenElement("error");
            p.PushAttribute(
                "message",
                fmt::format(
                    R"(compile error that was not a static_assert(..., "{}") - see <system-out>)",
                    run.tc.expected_assert_message)
                    .c_str());
            p.CloseElement();
        } else if (run.result() == test_case_result::compiled) {
            p.OpenElement("failure");
            p.PushAttribute(
                "message",
                fmt::format(
                    R"(code compiled - expected static_assert(..., "{}") - see <system-out>)",
                    run.tc.expected_assert_message)
                    .c_str());
            p.CloseElement();
        }

        if (ranges::contains(
                std::array{
                    test_case_result::compiled,
                    test_case_result::other_compile_failure,
                },
                run.result())) {

            p.OpenElement("system-out");
            log("out", "output", run.compiler_output->compile_output.stdout);
            p.PushText((run.compiler_output->compile_output.stdout | join('\n'))
                           .c_str(),
                       true);
            p.CloseElement();

            p.OpenElement("system-err");
            log("out", "error", run.compiler_output->compile_output.stderr);
            p.PushText((run.compiler_output->compile_output.stderr | join('\n'))
                           .c_str(),
                       true);
            p.CloseElement();
        }

        p.CloseElement();
    }
};

} // namespace dhagedorn::static_tester::priv