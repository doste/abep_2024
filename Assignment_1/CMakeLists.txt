cmake_minimum_required (VERSION 3.24.1)
project(genome_skew)

set(SOURCES
    find_oriC.c
)

add_library(findoriC SHARED ${SOURCES})