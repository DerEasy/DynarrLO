cmake_minimum_required(VERSION 3.20)
project(dynarrlo C)

set(CMAKE_VERBOSE_MAKEFILE ON)

add_library(dynarrlo dynarrlo.c dynarrlo.h)

target_compile_options(dynarrlo PRIVATE -Wall -Wpedantic -Wextra -O3 -std=c17)
