load("@hedron_compile_commands//:refresh_compile_commands.bzl", "refresh_compile_commands")

refresh_compile_commands(
    name = "compdb",
    exclude_external_sources = True,
    exclude_headers = "external",
    targets = {
        "//...": "",
    },
)

refresh_compile_commands(
    name = "compdb_codechecker",
    exclude_external_sources = True,
    exclude_headers = "all",
    targets = {
        "//test_runner:test_runner": "",
    },
)
