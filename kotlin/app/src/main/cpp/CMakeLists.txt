cmake_minimum_required(VERSION 3.22.1)

project("sdrradio")

# Configurar C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Adicionar diretórios de include
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/rtl-sdr/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/rtl-sdr/src)

# Configurar flags de compilação
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wall -Wextra")

# Definir variáveis para RTL-SDR (versão simplificada)
set(RTL_SDR_SOURCES
    rtl-sdr/src/tuner_e4k.c
    rtl-sdr/src/tuner_fc0012.c
    rtl-sdr/src/tuner_fc0013.c
    rtl-sdr/src/tuner_fc2580.c
    rtl-sdr/src/tuner_r82xx.c
)

# Criar biblioteca estática do RTL-SDR
add_library(rtlsdr STATIC ${RTL_SDR_SOURCES})

# Configurar flags específicas para Android
target_compile_definitions(rtlsdr PRIVATE
    ANDROID=1
    _REENTRANT=1
    _GNU_SOURCE=1
    LIBUSB_API_VERSION=0x01000100
)

# Criar biblioteca compartilhada principal
add_library(sdrradio SHARED
    sdr_radio_simple.cpp
)

# Linkar bibliotecas
target_link_libraries(sdrradio
    rtlsdr
    android
    log
    OpenSLES
)

# Configurar propriedades da biblioteca
set_target_properties(sdrradio PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
) 