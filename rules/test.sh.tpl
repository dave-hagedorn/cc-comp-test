#!/usr/bin/env bash

set -euo pipefail

# Bazel will set XML_OUTPUT_FILE
JUNIT="${XML_OUTPUT_FILE:-test.xml}"

extra_flags=()

# Bazel will set TEST_TMPDIR
if [[ "$TEST_TMPDIR" != "" ]]; then
    extra_flags+=("-t", "$TEST_TMPDIR")
fi

{TEST_RUNNER} -i "{INFO_BINARY}" -s "{SOURCE_FILE}" -c "{COMPILER_PATH}" -j "$JUNIT" "${extra_flags[*]}" --no-colour -- {ARGS}