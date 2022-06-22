load("@hedron_compile_commands//:refresh_compile_commands.bzl", "refresh_compile_commands")

refresh_compile_commands(
    name = "compdb_static_tester",
    exclude_external_sources = True,
    exclude_generated_sources = True,
    exclude_headers = "external",
    targets = {
        "//rules/static_tester:static_tester": "",
    },
)
