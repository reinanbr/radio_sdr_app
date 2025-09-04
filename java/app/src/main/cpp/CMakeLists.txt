cmake_minimum_required(VERSION 3.18.1)

project("radiosdr")

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required package for Android logging
find_library(log-lib log)

# Add librtlsdr source files
add_subdirectory(librtlsdr)

# Create our library
add_library(radiosdr SHARED
    radiosdr_jni.cpp
    sdr_controller.cpp
    signal_processor.cpp
    audio_processor.cpp
    spectrum_analyzer.cpp
    demodulator.cpp
)

# Include directories
target_include_directories(radiosdr PRIVATE
    librtlsdr/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link libraries
target_link_libraries(radiosdr
    ${log-lib}
    rtlsdr
    android
)

# Compiler specific options
target_compile_options(radiosdr PRIVATE
    -Wall
    -Wextra
    -O3
    -ffast-math
    -ftree-vectorize
)

if(${ANDROID_ABI} STREQUAL "arm64-v8a")
    target_compile_options(radiosdr PRIVATE -march=armv8-a)
elseif(${ANDROID_ABI} STREQUAL "armeabi-v7a")
    target_compile_options(radiosdr PRIVATE -march=armv7-a -mfpu=neon)
endif()
