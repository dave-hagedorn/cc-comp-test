cc_library(
    name = "support",
    srcs = [
        "support.cc",
    ],
    hdrs = [
        "support.hh",
    ],
    copts = [
        "--std=c++14",
    ],
    include_prefix = "static_test/support",
    visibility = ["//visibility:public"],
    deps = [
        "//lib:static_test",
    ],
)

cc_binary(
    name = "dev",
    srcs = [
        "dev.cc",
    ],
    copts = [
        "--std=c++14",
    ],
    deps = [
        "//lib:static_test",
        "//lib/private:support",
    ],
)
