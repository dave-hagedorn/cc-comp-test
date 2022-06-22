#pragma once

#include <future>
#include <regex>
#include <string>

#include "boost/asio.hpp"
#include "boost/filesystem.hpp"
#include "boost/process.hpp"
#include "boost/process/args.hpp"
#include "log.hh"
#include "range/v3/all.hpp"

namespace dhagedorn::static_tester::priv {

namespace bp = boost::process;
namespace bfs = boost::filesystem;
namespace rv = ranges::views;
namespace r = ranges;

struct executable_output {
    int exit_code;
    std::vector<std::string> stdout;
    std::vector<std::string> stderr;
};

struct executable {
    bfs::path path;
    std::vector<std::string> args;

    auto run() const {
        std::vector<std::string> prefix = {path.native()};

        auto cmd_line = ranges::views::concat(prefix, args)
                        | ranges::views::join(' ') | ranges::to<std::string>();

        // log("cmd line", "line", cmd_line);
        executable_output out;

        boost::asio::io_service ios;
        std::future<std::string> stdout, stderr;
        bp::child proc;

        try {
            proc = bp::child(
                path,
                bp::args(args),
                bp::std_err > stderr,
                bp::std_out > stdout,
                ios
            );

            ios.run();
            proc.wait();
        } catch (bp::process_error &err) {
            log("process error", "msg", err.what(), "code", err.code().value());
        }

        auto stderr_copy = stderr.get();
        auto stdout_copy = stdout.get();
        // log("output", "stdout", stdout_copy, "stderr", stderr_copy);
        out.stderr
            = stderr_copy | rv::split('\n') | r::to<std::vector<std::string>>();
        out.stdout
            = stdout_copy | rv::split('\n') | r::to<std::vector<std::string>>();

        out.exit_code = proc.exit_code();

        return out;
    }
};

} // namespace dhagedorn::static_tester::priv