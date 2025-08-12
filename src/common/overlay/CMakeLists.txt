add_library(common_overlay
    overlay.cpp
    effect_bar.cpp
    stat_bar.cpp
    shared_types.cpp
)

target_include_directories(common_overlay PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/common/core
    ${CMAKE_SOURCE_DIR}/src/common/game
    ${CMAKE_SOURCE_DIR}/src/er
    ${CMAKE_SOURCE_DIR}/src/er/structs
    ${CMAKE_SOURCE_DIR}/third_party/imgui
    ${CMAKE_SOURCE_DIR}/third_party/stb_image
)
