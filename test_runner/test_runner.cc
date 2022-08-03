
#include <chrono>
#include <stdio.h>

#include <cstdio>
#include <cstdlib>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"
#include "fmt/color.h"
#include "fmt/core.h"
#include "fmt/ranges.h"
#include "range/v3/all.hpp"

#include "code.hh"
#include "comp_test/comp_test.hh"
#include "compiler.hh"
#include "executable.hh"
#include "junit.hh"
#include "lib/comp_test.hh"
#include "log.hh"
#include "test_case_run.hh"
#include "test_suite.hh"
#include "test_suite_run.hh"

namespace dhagedorn::static_tester::priv {

using namespace std::chrono_literals;

namespace po = boost::program_options;
namespace r = ranges;
namespace rv = ranges::views;
namespace ra = ranges::actions;

struct args {
    std::string info_binary;
    std::string source;
    std::string compiler;
    std::optional<std::string> temp;
    std::optional<std::string> junit;
    bool colour;
    std::vector<std::string> compiler_args;

    void print() {
        log("args",
            "source",
            source,
            "compiler",
            compiler,
            "temp",
            temp,
            "junit",
            junit,
            "colour",
            colour,
            "compiler_args",
            compiler_args);
    }
};

auto validate_opts(const po::variables_map &result,
                   std::vector<std::string> positional) {
    auto error = [](auto msg) {
        fmt::print(
            stderr, fmt::emphasis::bold | fg(fmt::color::red), "{}\n", msg);
    };

    auto passed = true;

    auto check = [&](auto test, auto msg) {
        if (!test) {
            error(msg);
            passed = false;
        }
    };

    check(result.count("info") == 1,
          "-i, --info expected - info binary to list test cases");

    check(result.count("source") == 1,
          "-s,--source expected - source file to build under test");

    check(
        result.count("compiler") == 1,
        "-c,-compiler expected - path to compiler used to execute build tests");

    check(positional.size() > 0,
          "additional positional arguments expected - "
          "arguments to compiler (-c)");

    return passed;
}

auto parse_opts(int argc, char **argv) {
    po::options_description options{
        R"(Runner for static_test rule - invokes compiler for a source
        "file and determines if static_assert triggered at compile time)"};

    // clang-format off
    options.add_options()
        ("info,i", po::value<std::string>()->required(), "Info binary - compiled test suite with default main runner")
        ("source,s", po::value<std::string>()->required(), "Source file to build under test - checking for static_assert()")
        ("compiler,c", po::value<std::string>()->required(), "Path to compiler")
        ("temp,t", po::value<std::string>(), "Temp dir (defaults to system specified, but your build system may have another")
        ("junit,j", po::value<std::string>(), "Junit output file")
        ("no-colour", po::bool_switch()->default_value(false), "Disable colour in log output")
        ("help,h", "This menu")
    ;
    // clang-format on

    po::variables_map opts;

    auto parsed = po::command_line_parser(argc, argv)
                      .options(options)
                      .allow_unregistered()
                      .run();

    auto positional
        = collect_unrecognized(parsed.options, po::include_positional);

    if (opts.count("help")) {
        options.print(std::cout);
        std::exit(0);
    }

    po::variables_map parsed_opts;
    po::store(parsed, parsed_opts);

    if (!validate_opts(parsed_opts, positional)) {
        options.print(std::cerr);
        std::exit(1);
    };

    return args{
        parsed_opts["info"].as<std::string>(),
        parsed_opts["source"].as<std::string>(),
        parsed_opts["compiler"].as<std::string>(),
        opt_if(parsed_opts.count("temp")).then([&] {
            return parsed_opts["temp"].as<std::string>();
        }),
        opt_if(parsed_opts.count("junit")).then([&] {
            return parsed_opts["junit"].as<std::string>();
        }),
        !parsed_opts["no-colour"].as<bool>(),
        positional,
    };
}

auto run_case(const args &args, const dhagedorn::static_test::test_case &tc) {
    auto c = code(tc.file);

    auto start = std::chrono::steady_clock::now();

    fmt::print("{}\n", tc.symbol);

    auto runner = fmt::format(
        R"(
        struct TEST_INFO {{
            static constexpr const char* suite = "{}";
            static constexpr const char* object = "{}";
            static constexpr const char* verb = "{}";
            static constexpr const char* expected_static_assert = "{}";
            static constexpr const char* file = "{}";
            static constexpr unsigned line = {};
        }};

        int main() {{
            // instantiate test function, should static assert
            {}{}<TEST_INFO>();
            return 0;
        }}
    )",
        tc.test_suite(),
        tc.object,
        tc.verb,
        tc.expected_assert_message,
        tc.file,
        tc.line,
        tc.test_suite() == "" ? ""s : tc.test_suite() + "::",
        tc.symbol);

    c.append(runner);

    auto comp = compiler(args.compiler, args.compiler_args);

    auto result = comp.compile(c.as_file());

    auto duration = std::chrono::steady_clock::now() - start;

    return testcase_run{
        tc,
        result,
        std::chrono::duration_cast<std::chrono::milliseconds>(duration),
    };
}

auto run_suite(const args &args, const test_suite &suite) {
    auto runs = suite.test_cases | ranges::views::transform([&](auto &tc) {
                    return run_case(args, tc);
                })
                | ranges::to<std::vector>();

    return runs;
}

auto run_tests(const args &args, const std::vector<test_suite> &suites) {

    auto runs = suites | rv::transform([&](auto &ts) {
                    return test_suite_run{ts, run_suite(args, ts)};
                })
                | ranges::to<std::vector>();

    return runs;
}

auto get_tests(const args &args) {
    auto c = code(args.source);

    auto info = executable{args.info_binary};

    auto output = info.run();

    log("raw output", "output", output.stdout);

    auto sorted_verctor = r::to<std::vector>()
                          | ra::stable_sort([](const auto &a, const auto &b) {
                                return a.line < b.line;
                            });

    auto filter_and_strip = [](std::string_view prefix) {
        return rv::remove_if(
                   [&](auto &val) { return !(val.find(prefix) == 0); })
               | rv::transform(
                   [&](auto &val) { return val.substr(prefix.size()); });
    };

    auto test_suites
        = output.stdout | filter_and_strip("test_suite:")
          | rv::transform(&dhagedorn::static_test::test_suite::from_string)
          | sorted_verctor;

    auto test_cases
        = output.stdout | filter_and_strip("test_case:")
          | rv::transform(&dhagedorn::static_test::test_case::from_string)
          | sorted_verctor;

    return suites | rv::values | r::to<std::vector>();
}

auto write_junit(const args &args, const std::vector<test_suite_run> &runs) {
    if (args.junit) {
        junit j;
        j.write(runs, *args.junit);
    }
}

} // namespace dhagedorn::static_tester::priv

int main(int argc, char **argv) {
    dhagedorn::static_tester::priv::log(
        "env",
        "bin",
        argv[0],
        "pwd",
        boost::filesystem::current_path().native());

    auto args = dhagedorn::static_tester::priv::parse_opts(argc, argv);

    args.print();

    auto tests = get_tests(args);

    auto runs = run_tests(args, tests);

    write_junit(args, runs);

    auto passed
        = ranges::all_of(runs, [](auto &run) { return run.all_passed(); });

    return passed ? 0 : 1;
}