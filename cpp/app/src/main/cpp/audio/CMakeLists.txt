# CMakeLists.txt para processador de áudio
# Em uma implementação real, isso seria substituído por bibliotecas de processamento de áudio

add_library(audio_processor STATIC
    audio_processor.cpp)

target_include_directories(audio_processor PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})

# Configurações específicas para Android
set_target_properties(audio_processor PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON) 