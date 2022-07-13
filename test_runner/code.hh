#pragma once

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <strstream>

#include "fmt/core.h"

#include "boost/filesystem.hpp"

namespace dhagedorn::static_tester::priv {

namespace bfs = boost::filesystem;

class code {
public:
    code(std::string path)
        : _path{path}
        , _content{read_file()} {}

    void append(std::string content) { _content << content; }

    std::string content() { return _content.str(); }

    bfs::path as_file() const {
        bfs::temp_directory_path();

        // TODO:  Use TEST_TMPDIR if BAZEL_TEST set
        // https://bazel.build/reference/test-encyclopedia#initial-conditions
        // https://bazel.build/reference/test-encyclopedia#filesystem
        auto tmp = bfs::temp_directory_path()
                   / bfs::unique_path().replace_extension(".cc");

        std::ofstream fout{tmp.native()};

        fout << _content.rdbuf();

        return tmp;
    }

private:
    std::stringstream read_file() const {
        std::ifstream fin{_path};

        if (!fin.is_open()) {
            throw std::runtime_error{fmt::format("Could not open {}", _path)};
        }

        std::stringstream str;
        str << fin.rdbuf();

        return str;
    }

    std::string _path;
    std::stringstream _content;
};

} // namespace dhagedorn::static_tester::priv