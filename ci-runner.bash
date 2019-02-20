#!/usr/bin/env bash
# the 'MONADIC' define builds the monadic version if defined

# NOTE: pthread has to be linked in when compiling with gcc or else atomics are
# not guaranteed to carry the atomic overhead with gcc
# https://snf.github.io/2019/02/13/shared-ptr-optimization/
${CXX_COMPILER} -std=c++17 -Wall -Werror main.cpp -o monadic_main -DMONADIC=1
${CXX_COMPILER} -std=c++17 -Wall -Werror main.cpp -o normal_main

time ./monadic_main 100000
time ./normal_main 100000
