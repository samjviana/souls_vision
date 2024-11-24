project(imgui LANGUAGES C)

set(IMGUI_SOURCES
        "imgui.cpp"
        "imgui_draw.cpp"
        "imgui_tables.cpp"
        "imgui_widgets.cpp"
)
set(IMGUI_BACKEND_SOURCES
        "backends/imgui_impl_dx12.cpp"
        "backends/imgui_impl_win32.cpp"
)

add_library(imgui STATIC ${IMGUI_SOURCES})
add_library(imgui_backends STATIC ${IMGUI_BACKEND_SOURCES})

target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(imgui_backends PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})