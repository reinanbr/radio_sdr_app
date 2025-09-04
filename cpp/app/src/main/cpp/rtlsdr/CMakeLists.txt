# CMakeLists.txt para biblioteca RTL-SDR simulada
# Em uma implementação real, isso seria substituído pela biblioteca RTL-SDR real

add_library(rtlsdr STATIC
    rtlsdr_simulated.cpp)

target_include_directories(rtlsdr PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})

# Configurações específicas para Android
set_target_properties(rtlsdr PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON) 