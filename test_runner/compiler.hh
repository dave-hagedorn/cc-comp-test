#pragma once

#include <iostream>
#include <iterator>
#include <regex>
#include <string>
#include <unordered_map>

#include "boost/filesystem.hpp"
#include "boost/optional.hpp"
#include "fmt/format.h"
#include "range/v3/all.hpp"

#include "executable.hh"
#include "log.hh"

namespace dhagedorn::static_tester::priv {

namespace bfs = boost::filesystem;
namespace r = ranges;
namespace rv = ranges::views;

enum class severity {
    info,
    warning,
    error,
    unknown,
};

struct compiler_diagnostic {
    std::string original;

    bfs::path path;
    unsigned long line;
    unsigned long column;
    severity sev;
    std::string message;
    std::optional<std::string> static_assert_msg;

    inline const static std::unordered_map<std::string, severity>
        severity_words{
            {"note", severity::info},
            {"warning", severity::warning},
            {"error", severity::error},
        };

    static std::optional<compiler_diagnostic>
    from_string(const std::string &line) {
        std::regex re{R"(^([^:]+):(\d+):(\d+):\s*([^:\s]+)\s*:(.+)$)"};
        std::smatch match;

        if (!std::regex_match(line, match, re)) {
            return {};
        }

        return {{
            line,
            bfs::path{match[1]},
            std::stoul(match[2]),
            std::stoul(match[3]),
            severity_words.find(match[4]) != severity_words.cend()
                ? severity_words.at(match[4])
                : severity::unknown,
            match[5],
            find_static_assert(line),
        }};
    }

private:
    static std::optional<std::string>
    find_static_assert(const std::string &line) {
        const static std::vector<std::regex> tests{
            // clang: <source>:3:1: error: static_assert failed "msg"
            std::regex{R"_(static_assert failed "([^"]+)")_"},
            // clang: <source>:3:1 error: static_assert failed due to
            // requirement '<condition>' "msg"
            std::regex{
                R"_(static_assert failed due to requirement [^"]+"([^"]+))_"},
            // gcc: <source>:3:15: error: static assertion failed: msg
            std::regex{R"_(static assertion failed: (.*))_"},
            // msvc: <source>(3): error C2338: static_assert failed: 'msg'
            std::regex{R"_(static_assert failed: '([^']+)')_"},
        };

        for (auto &re : tests) {
            std::smatch m;
            if (std::regex_search(line, m, re)) {
                return m[1].str();
            }
        }

        return {};
    }
};

struct compile_result {
    bfs::path input;
    std::optional<bfs::path> binary;
    std::optional<executable> exec;
    executable_output compile_output;
    std::vector<compiler_diagnostic> diagnostics;
    bool compiled;

    bool has_static_assert(const std::string &msg) const {
        return r::any_of(diagnostics, [&](auto &diag) {
            return diag.static_assert_msg == msg;
        });
    }

    bool did_static_assert() const {
        return r::any_of(diagnostics, [&](auto &diag) {
            return diag.static_assert_msg.has_value();
        });
    }

    std::optional<std::string> static_assert_msg() const {
        if (!did_static_assert()) {
            return {};
        }

        return r::find_if(diagnostics,
                          [](auto &diag) {
                              return diag.static_assert_msg.has_value();
                          })
            ->static_assert_msg;
    }
};

class compiler {
public:
    compiler(std::string path, std::vector<std::string> args)
        : _path{path}
        , _args{args} {}

    compile_result compile(bfs::path input) {
        auto output = bfs::temp_directory_path()
                      / bfs::unique_path().replace_extension(".o");

        executable exec{_path, _rewrite_args(_args, input, output)};

        compile_result comp_result;

        comp_result.compile_output = exec.run();

        log("comp output",
            "stdout",
            comp_result.compile_output.stdout,
            "stderr",
            comp_result.compile_output.stderr);

        if (bfs::is_regular(output)) {
            bfs::permissions(output,
                             bfs::perms::owner_exe | bfs::perms::owner_read
                                 | bfs::perms::owner_write);
        }

        comp_result.input = input;

        comp_result.diagnostics
            = rv::concat(comp_result.compile_output.stderr,
                         comp_result.compile_output.stdout)
              | rv::transform(&compiler_diagnostic::from_string)
              | rv::remove_if(
                  [](const auto &diag) { return !diag.has_value(); })
              | rv::transform([](const auto &diag) { return *diag; })
              | r::to<std::vector>();

        comp_result.exec = executable{output, {}};

        if (comp_result.compile_output.exit_code == 0) {
            comp_result.binary = output;
            comp_result.compiled = true;
        } else {
            comp_result.compiled = false;
        }

        return comp_result;
    }

private:
    std::vector<std::string> _rewrite_args(const std::vector<std::string> &args,
                                           const bfs::path &input,
                                           const bfs::path &output) {
        auto rewritten = args;
        //= _args | rv::remove_if([](const auto &e) { return e == "-c"; })
        //  | r::to<std::vector>();

        std::regex is_cc{R"(.*\.(c|cc|cpp))"};

        bool wrote_input = false;
        bool wrote_output = false;
        bool wrote_compile_only = false;

        std::string prev;
        for (auto &arg : rewritten) {
            if (std::regex_match(arg, is_cc)) {
                arg = input.native();
                wrote_input = true;
            } else if (prev == "-o") {
                arg = output.native();
                wrote_output = true;
            } else if (arg == "-c") {
                wrote_compile_only = true;
            }

            prev = arg;
        }

        if (!wrote_output) {
            rewritten.push_back("-o");
            rewritten.push_back(output.native());
        }

        if (!wrote_input) {
            rewritten.push_back(input.native());
        }

        if (!wrote_compile_only) {
            rewritten.push_back("-c");
        }

        // Should work for clang and gcc
        rewritten.push_back("-fdiagnostics-color=never");

        // log("rewritten args", "rewritten", rewritten);

        return rewritten;
    }

    std::string _path;
    std::vector<std::string> _args;
};

} // namespace dhagedorn::static_tester::priv

template <>
struct fmt::formatter<dhagedorn::static_tester::priv::compiler_diagnostic>
    : fmt::formatter<std::string> {
    template <typename FormatContext>
    auto format(const dhagedorn::static_tester::priv::compiler_diagnostic &p,
                FormatContext &ctx) const {
        // ctx.out() is an output iterator to write to.
        return format_to(ctx.out(), "{}", p.original);
    }
};