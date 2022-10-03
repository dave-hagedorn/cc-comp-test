- [cc_test, but for static_assert](#cc_test-but-for-static_assert)
- [Installation](#installation)
- [Usage](#usage)
- [Detailed Documentation](#detailed-documentation)
  - [cc_comp_test](#cc_comp_test)
  - [comp_test.hh](#comp_testhh)
  - [JUnit Output (test.xml)](#junit-output-testxml)
- [Hacking/Contributing](#hackingcontributing)
- [Coming Features - Vote!](#coming-features---vote)
- [How it Works](#how-it-works)


# cc_test, but for static_assert

Unit test your C++ `static_assert()`'s in Bazel with `cc_comp_test`

`cc_comp_test` is a Bazel rule to build and run unit tests that validate static_assert behaviour.


# Installation

```py
# WORKSPACE

http_archive(
    name = "cc_comp_test",
    sha256 = "",
    strip_prefix = "cc_comp_test-1.0.0",
    url = "https://www.github.com/dave-hagedorn/cc_comp_test/archive/1.0.0.zip",
)
```

# Usage

Write your test cases in any source file

```c++
// numbers_test.cc

#include <type_traits>

#include "comp_test/comp_test.hh"

template <typename T>
auto picky_to_string(T &&value) {
    static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>, "only numbers are supported");

    if constexpr (std::is_integral_v<T>) {
        static_assert(std::is_unsigned_v<T>, "signed types not supported");
    }

    return std::to_string(value);
}

// Test cases can be grouped inside test suites
TEST_SUITE("numbers") {

    TEST_COMP_ASSERT("to_string", "only allows numbers", "only numbers are supported") {
        auto value = "invalid value";

        picky_to_string(value);
    }

    // Designated initializer notation can also be used, allowing for "named" arguments
    // You need -std=c++20  for this, otherwise you may see compile warnings
    // Clang/GCC also support this as an extension pre-C++20
    TEST_COMP_ASSERT(.thing="to_string", .will="does not allow signed types", .assert_with="signed types not supported") {
        picky_to_string(-3);
    }
}
```

Use the cc_comp_test rule to define a test target using these test cases

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
| src       | Source file containing your test cases.  Optional - `<name>.cc` is assumed if `src` is not defined                                                                |
| deps      | any deps required to compile your test - your lib under test, other libs, etc.  Usually other cc_library()'s, but any rule with a provider of `CcInfo` is allowed |

## comp_test.hh

Supported test definitions

| name                                       | meaning                                                                                                              |
|--------------------------------------------|----------------------------------------------------------------------------------------------------------------------|
| TEST_SUITE(name)                           | Use to group test cases.  Symbols defined within a `TEST_SUITE` are scoped to that suite only                        |
| TEST_COMP_ASSERT(thing, will, assert_with) | Define a test case with code that must fail a `static_assert` as `static_assert(<evaluate-to-false>, "assert_with")` |

Test functions/macros accept named arguments as C++20 designated initializers.  This seems to work OK with clangd-based completion so I decided to kee it.

## JUnit Output (test.xml)

Each test run will emit a test.xml in JUnit format, including some extra attributes Bazel seems to like to add.

Each case can pass, fail, or error, with the meaning of this depending on the test case type

| test case type   | pass                                                         | fail                                                 | error                                   |
|------------------|--------------------------------------------------------------|------------------------------------------------------|-----------------------------------------|
| TEST_COMP_ASSERT | compilation failed with the expected `static_assert` message | compilation succeeded - `static_assert` did not fire | compilation failed for any other reason |


# Hacking/Contributing

If you use VSCode and are OK to work in a [Dev Container](https://code.visualstudio.com/docs/remote/containers), this is the recommended approach.

As usual just open this folder in VSCode and when prompted, reopen inside the Dev Container.

The recommended extensions such as clangd for code navigation will be installed.

As much as possible workspace settings are configured in `.vscode/settings.json` rather than `.devcontainer/devcontainer.json`, this way settings are applied regardless of
whether this workspce is open inside a Dev Container or not.


# Coming Features - Vote!

https://github.com/apex/gh-polls#about

[![](https://api.gh-polls.com/poll/01G6X5273KW1PPR48JG522SYYR/a)](https://api.gh-polls.com/poll/01G6X5273KW1PPR48JG522SYYR/a/vote)
<br/>
[![](https://api.gh-polls.com/poll/01G6X5273KW1PPR48JG522SYYR/b)](https://api.gh-polls.com/poll/01G6X5273KW1PPR48JG522SYYR/b/vote)
<br/>
[![](https://api.gh-polls.com/poll/01G6X5273KW1PPR48JG522SYYR/c)](https://api.gh-polls.com/poll/01G6X5273KW1PPR48JG522SYYR/c/vote)

This is currently focused on static_assert statements, but will be expanded to support additional test cases:

- [ ] Other forms of "static assert" - such as `throw` in a constexpr context
- [ ] Generic "it just compiles" test case
- [ ] Generic "must not compile" test case (do not validate type of assertion or its message)
- [ ] Parameterized tests - still TBD - but ability to run a test case across multiple types and/or compile-time values (somehow)


# How it Works

Assuming your test suites and cases for one `cc_comp_test` target are defineed in a `test.cc`, 
running tests in `test.cc` involves the following.

First, the test code must be built.

1. For each test case in `test.cc` the `TEST_COMP_ASSERT` macro creates a unique, templated, test function
2.  `TEST_COMP_ASSERT` also generates info about each test case's attributes - it's name, the test case object/class and verb, etc.
3. `test.cc` is linked with a predefined `main.cc` that uses this info to print out all the test cases and their attributes
   * This is the `info binary` - one of the two outputs of any `cc_comp_test` target
4. Next, another binary that is responsible for running all of the cases in `test.cc` is built
   * This is the `test runner` - this is the second output of any `cc_comp_test` target
   * Unlike the `info binary` this is only built once, and used across all `cc_comp_test` targets

After all of the prerequisites have been built the tests can be run:

1.  The `test runner` now runs the `info binary` and captures the info it prints out - the test cases and their attributes
2.  The `test runner` parses this info to build a list of the cases in `test.cc` - the test function names and other case attributes
3.  For each case, the `test runner` will do a test compile:
    1.  It generates a `main()` function that instantiates the cases's templated test function
    2.  The runner then copies `test.cc` to a new file `test'.cc` (this is not the actual name used), appending this new `main()` function
    3.  The runner then tries to compile `test'.cc`
    4.  If the compilation fails with an error message matching the expected `static_assert()` message for this case, the test case has passed
    5.  If there is another compilation error or the compilation passes, the case is considered failed


