cmake_minimum_required(VERSION 3.27)
project(souls_vision)

set(CMAKE_CXX_STANDARD 23)

# Define the path to the game and mods folder
set(GAME_DIR "C:/Program Files (x86)/Steam/steamapps/common/ELDEN RING/Game")
set(MODS_DIR "${GAME_DIR}/mods")
set(GAME_EXE "ersc_launcher.exe")

file(COPY ${CMAKE_SOURCE_DIR}/sv_assets DESTINATION ${CMAKE_BINARY_DIR})

add_library(souls_vision SHARED
        dll_main.cpp
        globals.cpp
        globals.h
        game_handler.cpp
        game_handler.h
        hook_helper.h
        hook_helper.cpp
        memory.cpp
        memory.h
        overlay.h
        overlay.cpp
        logger.cpp
        logger.h
        config.cpp
        config.h
        structs/world_chr_man_imp.cpp
        structs/world_chr_man_imp.h
        structs/chr_ins.cpp
        structs/chr_ins.h
        structs/entry.cpp
        structs/entry.h
        structs/chr_module_bag.cpp
        structs/chr_module_bag.h
        structs/chr_stat_module.cpp
        structs/chr_stat_module.h
        structs/chr_resist_module.cpp
        structs/chr_resist_module.h
        structs/chr_super_armor_module.cpp
        structs/chr_super_armor_module.h
        shared_types.cpp
        shared_types.h
        util.cpp
        util.h
        stat_bar.cpp
        stat_bar.h
        effect_bar.cpp
        effect_bar.h
)

target_link_libraries(souls_vision PRIVATE d3d12 dxgi dxguid)
add_subdirectory(third_party)

add_custom_command(TARGET souls_vision POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:souls_vision>" "${MODS_DIR}/"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/sv_assets" "${MODS_DIR}/sv_assets"
)
