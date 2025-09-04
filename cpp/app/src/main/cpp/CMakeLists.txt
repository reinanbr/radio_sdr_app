cmake_minimum_required(VERSION 3.22.1)

project("sdrradio")

# Configurar C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Encontrar bibliotecas necessárias
find_library(log-lib log)
find_library(android-lib android)
find_library(OpenSLES-lib OpenSLES)

# Incluir diretórios
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Adicionar subdiretórios para bibliotecas externas
add_subdirectory(rtlsdr)
add_subdirectory(audio)

# Criar biblioteca compartilhada principal
add_library(sdrradio SHARED
    sdr_radio.cpp
    sdr_manager.cpp
    audio_manager.cpp
    usb_manager.cpp)

# Linkar bibliotecas
target_link_libraries(sdrradio
    ${log-lib}
    ${android-lib}
    ${OpenSLES-lib}
    rtlsdr
    audio_processor)

# Configurações específicas para Android
set_target_properties(sdrradio PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON) 