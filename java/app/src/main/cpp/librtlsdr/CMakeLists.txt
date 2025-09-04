cmake_minimum_required(VERSION 3.18.1)

project(rtlsdr)

# Set C standard
set(CMAKE_C_STANDARD 99)

# Add library
add_library(rtlsdr STATIC
    src/rtl-sdr.c
    src/tuner_r82xx.c
    src/convenience.c
)

# Include directories
target_include_directories(rtlsdr PUBLIC
    include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Compile definitions
target_compile_definitions(rtlsdr PRIVATE
    __ANDROID__
    _GNU_SOURCE
)

# Compiler options
target_compile_options(rtlsdr PRIVATE
    -Wall
    -Wextra
    -O2
)
