# Define the path to the game and mods folder
set(GAME_DIR "C:/Program Files (x86)/Steam/steamapps/common/ELDEN RING/Game")
set(MODS_DIR "${GAME_DIR}/mods")
set(GAME_EXE "ersc_launcher.exe")

file(COPY ${CMAKE_SOURCE_DIR}/sv_assets DESTINATION ${CMAKE_BINARY_DIR})

add_library(souls_vision SHARED
    dll_main.cpp
    game_handler.cpp
    hook_helper.cpp
    renderer.cpp
    structs/chr_ins.cpp
    structs/chr_module_bag.cpp
    structs/chr_resist_module.cpp
    structs/chr_stat_module.cpp
    structs/chr_super_armor_module.cpp
    structs/cs_menu_man_imp.cpp
    structs/entry.cpp
    structs/fd4_basic_hash_string.cpp
    structs/loading_screen_data.cpp
    structs/npc_param.cpp
    structs/param_data.cpp
    structs/param_header.cpp
    structs/param_res_cap.cpp
    structs/param_row.cpp
    structs/solo_param_holder.cpp
    structs/solo_param_repository_imp.cpp
    structs/world_chr_man_imp.cpp
)

# Include directories
 target_include_directories(souls_vision PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/structs
    ${CMAKE_SOURCE_DIR}/src/common/core
    ${CMAKE_SOURCE_DIR}/src/common/game
    ${CMAKE_SOURCE_DIR}/src/common/overlay
 )

# Link libraries
 target_link_libraries(souls_vision PRIVATE
    common_core
    common_game
    common_overlay
    d3d12 dxgi dxguid
 )

add_custom_command(TARGET souls_vision POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:souls_vision>" "${MODS_DIR}/"
)

target_sources(souls_vision PRIVATE ${CMAKE_SOURCE_DIR}/src/common/overlay/resources.rc)
set_source_files_properties(${CMAKE_SOURCE_DIR}/src/common/overlay/resources.rc PROPERTIES LANGUAGE RC)
