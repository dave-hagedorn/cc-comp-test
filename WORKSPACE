local_repository(
    name = "bazel-utils_",
    path = "/workspaces/bazel-utils",
)

# Common setup
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "bazel-utils",
    sha256 = "da3d62e2ae11b37cbae92bc31c6c260494831a9cc7e33a6f1702308686426c2a",
    strip_prefix = "bazel-utils-1.2.0",
    url = "https://www.github.com/dave-hagedorn/bazel-utils/archive/1.2.0.zip",
)

load("@bazel-utils//:bazel-utils.bzl", "LLVM_VERSION", "cc_workspace_dependencies", "github_archive")

cc_workspace_dependencies()

# Macros can't load(), so load rules/macros fetched in cc_dependencies()
load("@hedron_compile_commands//:workspace_setup.bzl", "hedron_compile_commands_setup")
load("@com_grail_bazel_toolchain//toolchain:deps.bzl", "bazel_toolchain_dependencies")
load("@com_grail_bazel_toolchain//toolchain:rules.bzl", "llvm_toolchain")

# It would be better if these were in one convenience macro, but macos cannot contain undefined symbols
# - these calls have to appear after the load()'s above
hedron_compile_commands_setup()

bazel_toolchain_dependencies()

llvm_toolchain(
    name = "llvm_toolchain",
    llvm_version = LLVM_VERSION,
)

load("@llvm_toolchain//:toolchains.bzl", "llvm_register_toolchains")

llvm_register_toolchains()
# End common setup

github_archive(
    org = "ericniebler",
    repo = "range-v3",
    sha256 = "",
    version = "0fa54d7de5dc00655e38a08e87cda61f7aa6d5b9",
)

github_archive(
    non_bazel = True,
    org = "fmtlib",
    repo = "fmt",
    sha256 = "3c75de0c43e899a7581d7304cf01c152a924750def179f66ee0b94a3e4fdc5db",
    version = "65dd2ea52c23721660cf027525c6640f868381f7",
)

github_archive(
    org = "nelhage",
    override_name = "com_github_nelhage_rules_boost",
    repo = "rules_boost",
    sha256 = "4ab5cec670be08a94f0683fe0c3a65dd18868e24bdf164f324566660fce8dab7",
    version = "652b21e35e4eeed5579e696da0facbe8dba52b1f",
)

load("@com_github_nelhage_rules_boost//:boost/boost.bzl", "boost_deps")

boost_deps()

github_archive(
    non_bazel = True,
    org = "leethomason",
    repo = "tinyxml2",
    sha256 = "",
    version = "e45d9d16d430a3f5d3eee9fe40d5e194e1e5e63a",
)
