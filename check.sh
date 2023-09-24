#!/bin/bash

CHECK_FILE=$1
test -f "compile_commands.json" || exit 1
clang-tidy ${CHECK_FILE} || exit 1
#/opt/homebrew/opt/llvm/bin/clang-check ${CHECK_FILE} || exit 1
cppcheck -q --std=c11 --check-level=exhaustive --inconclusive --enable=all --suppress=missingIncludeSystem --project=compile_commands.json || exit 1
flawfinder -Q --singleline --dataonly --minlevel=0 ${CHECK_FILE} || exit 1
