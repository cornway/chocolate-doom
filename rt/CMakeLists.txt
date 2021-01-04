add_library(rt STATIC
            rt.cpp        rt.h)

target_include_directories(rt PRIVATE "../" "${CMAKE_CURRENT_BINARY_DIR}/../../")
target_link_libraries(rt SDL2::SDL2 SDL2::mixer SDL2::net)
