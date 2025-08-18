add_library(common_game
    globals.cpp
)

target_include_directories(common_game PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(common_game PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party/imgui
)