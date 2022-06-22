#!/usr/bin/env bash

set -euo pipefail

#env
#echo $PWD

JUNIT="${XML_OUTPUT_FILE:-test.xml}"

{STATIC_TESTER} -i "{INFO_BINARY}" -s "{SOURCE_FILE}" -c "{COMPILER}" -j "$JUNIT" -- {ARGS}