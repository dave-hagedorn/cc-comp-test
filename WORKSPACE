local_repository(
    name = "bazel-utils",
    path = "/workspaces/bazel-utils",
)

load("@bazel-utils//:bazel-utils.bzl", "github_archive", "setup_compile_commands")

github_archive(
    org = "grailbio",
    override_name = "com_grail_bazel_toolchain",
    repo_name = "bazel-toolchain",
    sha256 = "1c813e5ede66901f6ab431f28640aba1590bca28ba9e8dc97661593b41d5dcdf",
    version = "0.7.2",
)

load("@com_grail_bazel_toolchain//toolchain:deps.bzl", "bazel_toolchain_dependencies")

bazel_toolchain_dependencies()

load("@com_grail_bazel_toolchain//toolchain:rules.bzl", "llvm_toolchain")

llvm_toolchain(
    name = "llvm_toolchain",
    llvm_version = "14.0.0",
)

load("@llvm_toolchain//:toolchains.bzl", "llvm_register_toolchains")

llvm_register_toolchains()

setup_compile_commands()

github_archive(
    org = "ericniebler",
    repo_name = "range-v3",
    sha256 = "",
    version = "0fa54d7de5dc00655e38a08e87cda61f7aa6d5b9",
)

github_archive(
    non_bazel = True,
    org = "fmtlib",
    repo_name = "fmt",
    sha256 = "3c75de0c43e899a7581d7304cf01c152a924750def179f66ee0b94a3e4fdc5db",
    version = "65dd2ea52c23721660cf027525c6640f868381f7",
)

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

github_archive(
    name = "com_github_nelhage_rules_boost",
    org = "nelhage",
    override_name = "com_github_nelhage_rules_boost",
    repo_name = "rules_boost",
    sha256 = "4ab5cec670be08a94f0683fe0c3a65dd18868e24bdf164f324566660fce8dab7",
    version = "652b21e35e4eeed5579e696da0facbe8dba52b1f",
)

load("@com_github_nelhage_rules_boost//:boost/boost.bzl", "boost_deps")

boost_deps()

github_archive(
    non_bazel = True,
    org = "leethomason",
    repo_name = "tinyxml2",
    sha256 = "",
    version = "e45d9d16d430a3f5d3eee9fe40d5e194e1e5e63a",
)
