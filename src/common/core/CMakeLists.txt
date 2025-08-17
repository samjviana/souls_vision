add_library(common_core
    config.cpp
    logger.cpp
    memory.cpp
    util.cpp
)

target_include_directories(common_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(common_core PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party/nlohmann_json/single_include
    ${CMAKE_SOURCE_DIR}/third_party/tomlplusplus
    ${CMAKE_SOURCE_DIR}/third_party/imgui
    ${CMAKE_SOURCE_DIR}/src/common/overlay
    ${CMAKE_SOURCE_DIR}/src/common/game
)
