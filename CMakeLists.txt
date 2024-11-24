cmake_minimum_required(VERSION 3.27)
project(souls_vision)

set(CMAKE_CXX_STANDARD 23)

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
        bar_renderer.cpp
        bar_renderer.h
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
        effect_bar_renderer.cpp
        effect_bar_renderer.h
)

target_link_libraries(souls_vision PRIVATE d3d12 dxgi dxguid)

add_subdirectory(third_party)
