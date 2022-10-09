- [cc_test, but for static_assert](#cc_test-but-for-static_assert)
- [Installation](#installation)
- [Usage](#usage)
- [Detailed Documentation](#detailed-documentation)
  - [cc_comp_test](#cc_comp_test)
  - [comp_test.hh](#comp_testhh)
  - [JUnit Output (test.xml)](#junit-output-testxml)
- [How it Works](#how-it-works)
- [Hacking/Contributing](#hackingcontributing)
  - [Dev Continer](#dev-continer)


# cc_test, but for static_assert

Unit test your C++ 11/14/17/20/2[3b] `static_assert()`'s and other compile time checks with Bazel.

`cc_comp_test` is a Bazel rule to build and run compile time tests.  It works with Bazel alongside your
cc_test and other cc_* rules, and emits Bael-friendly JUnit test results similar to GoogleTest.

`cc_comp_test` will work with Clang.

Tested on Linux.  GCC and MSVC support pending.

The core implementation does not require Bazel and should be portable to other build systems.

`cc_comp_test` itself is written in C++17

# Installation

Add `cc_comp_test` to your WORKSPACE:

```py
http_archive(
    name = "cc_comp_test",
    sha256 = "",
    strip_prefix = "cc_comp_test-1.0.0",
    url = "https://www.github.com/dave-hagedorn/cc_comp_test/archive/1.0.0.zip",
)
```

# Usage

Write your test cases in any single source file.

See [readme_sample.cc](readme_sample/readme_sample.cc) for full sample code.

```c++
#include <string>
#include <type_traits>
#include <vector>

#include "comp_test/comp_test.hh"

template <typename T>
void to_string(T &&value) {
    using decayed_t = typename std::decay<T>::type;
    static_assert(std::is_integral<decayed_t>{});
    std::to_string(value);
}

// ℹ️ [1] test cases can be listed standalone

// ℹ️ [2] test for a static_assert with a specific message
MUST_STATIC_ASSERT("to_string", "only works on numbers", "type not supported") {
    to_string(TestCase::object);
}

// ℹ️ [3] test that code compiles - speicifically that it does not static_assert
MUST_COMPILE("to_string", "only works on numbers") {

    // ℹ️ [4] test information is passed in as a TestCase object
    to_string(TestCase::line);
}

// ℹ️ [5] alternatively, test cases can be grouped into suites
TEST_SUITE("test_types", "should all pass") {

    // ℹ️ [6] arguments can be named using designated initializer synax
    MUST_STATIC_ASSERT(.object = "to_string",
                       .will = "only works on numbers",
                       .assert_with = "non-existent assert", ) {
        to_string(TestCase::object);
    }
}
```

Use the cc_comp_test rule to define a test target using these test cases

See [BUILD.bazel](readme_sample/BUILD.bazel) in [readme_sample](readme_sample) for full example.

```py
# BUILD.bazel

load("@cc_comp_test:cc_comp_test.bzl", "cc_comp_test")

cc_comp_test(
    name = "testing_numbers"
    # src can be omitted - <name>.cc will be used
    src = "testing_numbers.cc",
    # any normal cc_* deps are supported - cc_library(), etc.
    deps = [
        "//needed_lib",
    ]
)
```

```bash
bazel test :testing_numbers
```

# Detailed Documentation

## cc_comp_test

| parameter | meaning                                                                                                                                                           |
|-----------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `src`     | Source file containing your test cases.  Optional - `<name>.cc` is assumed if `src` is not defined                                                                |
| `deps`    | Any deps required to compile your test - your lib under test, other libs, etc.  Usually other cc_library()'s, but any rule with a provider of `CcInfo` is allowed |
| `copts` | Same as `copts` flag in any other `cc_*` rule - compiler flags to use when compiling your `.cc/.cpp/.cxx` files

## comp_test.hh

Supported test definitions

| name                                          | meaning                                                                                                              |
|-----------------------------------------------|----------------------------------------------------------------------------------------------------------------------|
| `TEST_SUITE(name)`                            | Use to group test cases.  Symbols defined within a `TEST_SUITE` are scoped to that suite only                        |
| `TEST_MUST_ASSERT(object, will, assert_with)` | Define a test case with code that must fail a `static_assert` as `static_assert(<evaluate-to-false>, "assert_with")` |
| `TEST_MUST_COMPILE(object, description)`      | Define a test case with code that must not `static_assert`                                                           |

Test functions/macros accept named arguments as C++20 designated initializers.  This seems to work OK with clangd-based completion so I decided to kee it.

## JUnit Output (test.xml)

Each test run will emit a test.xml in JUnit format, including some extra attributes Bazel seems to like to add.

Each case can pass, fail, or error, with the meaning of this depending on the test case type

| test case type     | pass                                                         | fail                                                                                     | error                                                                        |
|--------------------|--------------------------------------------------------------|------------------------------------------------------------------------------------------|------------------------------------------------------------------------------|
| `TEST_MUST_ASSERT` | compilation failed with the expected `static_assert` message | compilation succeeded - `static_assert` did not fire, a different `static_assert` fired. | compilation failed for any other reason - compile error, not `static_assert` |
| `TEST_MUST_COMPIL` | compilation succeeded                                        | compilation failed with some `static_assert`                                             | compilation failed for any other reason - compile error, not `static_assert` |


# How it Works

Assuming your test suites and cases for one `cc_comp_test` target are defineed in a `test.cc`,
running tests in `test.cc` involves the following.

First, the test code must be built.

1. For each test case in `test.cc` the `TEST_MUST_ASSERT` or `TEST_MUST_COMPILE` macros create a unique, templated, test function
2.  These macros also generates info about each test case's attributes - it's name, the type of test case (assert vs compile) and its object/class and verb, etc.
3. `test.cc` is linked with a predefined `main.cc` that uses this info to print out/serialize to stdout all the test cases and their attributes
   * This is the `info binary` - one of the two outputs of any `cc_comp_test` target
4. Next, another binary that is responsible for running all of the cases in `test.cc` is built
   * This is the `test runner` - this is the second output of any `cc_comp_test` target
   * Unlike the `info binary` this is only built once, and used across all `cc_comp_test` targets

After all of the prerequisites have been built the tests can be run:

1.  The `test runner` now runs the `info binary` and captures the info it prints out - the test cases and their attributes
2.  The `test runner` parses this info to build a list of the cases in `test.cc` - the test function names and other case attributes
3.  For each case, the `test runner` will do a test compile:
    1.  It generates a `main()` function that instantiates the cases's templated test function
    2.  The runner then copies `test.cc` to a new temp file `test'.cc` (this is not the actual name used), appending this new `main()` function
    3.  The runner then tries to compile `test'.cc` and parses any compiler output, including any `static_assert`ions and their messages
    4.  The test case's status is determined by type of test case, whether its compilation passed, failed with an expected `static_assert`, or failed with an unexpected static_assert or other compiler error

Note that using this approach, your test cases are isolated through templated functions.  As each test case - function - is instantiated only when it is "run", this causes any depdendant code in the test case to be evaluated at compile time with respect to `static_assert` or other compile time checks.

This does mean that invalid C++ code - improper syntax, etc - in one test csae is *not* isolated from other test cases and will cause all code to
fail to compile.  This will likly mean your test itself cannot be built - the `info binary` will fail to build in the first place.

# Hacking/Contributing

## Dev Continer

If you use VSCode and are OK to work in a [Dev Container](https://code.visualstudio.com/docs/remote/containers), this is the recommended approach.

Just open this folder in VSCode and when prompted, reopen inside the Dev Container.

The recommended extensions such as clangd for code navigation will be installed.
The clangd extension will ask you if it should download the clangd binary - just say yes.

As much as possible workspace settings are configured in `.vscode/settings.json` rather than `.devcontainer/devcontainer.json`, this way settings are applied regardless of whether this workspce is open inside a Dev Container or not.

:information_source: Note that devcontainer.json mounts some files from your $HOME into the container:

* ~/.gitconfig, so Git still works in the VS Code terminal, and especially that your user.email config is correct
* ~/.zshrc and/or ~/.bashrc so your shell is familiar
* ~/.bash_history and/or ~/.zhistory so commands are kept in your history file

If this makes you uncomfortable you can uncomment these lines in [devcontainer.json](.devcontainer/devcontainer.json)



