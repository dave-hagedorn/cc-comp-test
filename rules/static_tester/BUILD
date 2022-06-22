cc_binary(
    name = "static_tester",
    srcs = [
        "code.hh",
        "compiler.hh",
        "executable.hh",
        "junit.hh",
        "log.hh",
        "static_tester.cc",
        "test_case_run.hh",
        "test_suite.hh",
        "test_suite_run.hh",
        "util.hh",
    ],
    copts = [
        "--std=c++17",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//lib:static_test",
        "@boost//:asio",
        "@boost//:filesystem",
        "@boost//:process",
        "@boost//:program_options",
        "@fmt",
        "@range-v3",
        "@tinyxml2",
    ],
)
