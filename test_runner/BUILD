cc_binary(
    name = "test_runner",
    srcs = [
        "code.hh",
        "compiler.hh",
        "executable.hh",
        "junit.hh",
        "log.hh",
        "test_case_run.hh",
        "test_runner.cc",
        "test_suite.hh",
        "test_suite_run.hh",
        "util.hh",
    ],
    copts = [
        "--std=c++17",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//lib:comp_test",
        "@boost//:asio",
        "@boost//:filesystem",
        "@boost//:process",
        "@boost//:program_options",
        "@fmt",
        "@range-v3",
        "@tinyxml2",
    ],
)
