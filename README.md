- [cc_test, but for static_assert](#cc_test-but-for-static_assert)
- [Installation](#installation)
- [Usage](#usage)
- [Detailed Documentation](#detailed-documentation)
  - [cc_comp_test](#cc_comp_test)
  - [comp_test.hh](#comp_testhh)
  - [JUnit Output (test.xml)](#junit-output-testxml)
- [Hacking/Contributing](#hackingcontributing)
  - [Dev Continer](#dev-continer)
- [Coming Features - Vote!](#coming-features---vote)
- [How it Works](#how-it-works)


# cc_test, but for static_assert

Unit test your C++ 11/14/17/20 `static_assert()`'s and other compile time checks with Bazel.

`cc_comp_test` is a Bazel rule to build and run compile time tests.  It works with Bazel alongside your 
cc_test and other testing rules, and emits JUnit test results.

`cc_comp_test` will work with Clang, GCC, and MSVC using C++11 or newer code.

The core implementation does not require Bazel and should be portable to other build systems.

`cc_comp_test` itself is written in C++17

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

Write your test cases in any single source file

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

// test cases can be listed standalone

// test for a static_assert with a specific message
MUST_STATIC_ASSERT("to_string", "only works on numbers", "type not supported") {
    to_string(TestCase::object);
}

// test that code does not static_assert
MUST_COMPILE("to_string", "only works on numbers") {

    // test information is passed in as a TestCase object
    to_string(TestCase::line);
}

// alternatively, test cases can be grouped into suites
TEST_SUITE("test_types", "should all pass") {

    // arguments can be named using designated initializer synax
    MUST_STATIC_ASSERT(.object = "to_string",
                       .will = "only works on numbers",
                       .assert_with = "non-existent assert", ) {
        to_string(TestCase::object);
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


