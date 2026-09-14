# Vendored third-party sources.
#
# Kept as its own target so these files are not compiled with the project's
# warning flags, and so their headers are consumed as SYSTEM includes by
# everything downstream.

add_library(third_party STATIC
    imgui/imgui_impl_sdl2.cpp
    imgui/imgui_impl_sdlrenderer2.cpp
)

target_include_directories(third_party SYSTEM PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# The imgui backends need SDL2 and imgui itself; imgui_entt_entity_editor.hpp
# needs EnTT. PUBLIC because they appear in the headers this target exposes.
target_link_libraries(third_party PUBLIC
    SDL2::SDL2
    imgui::imgui
    EnTT::EnTT
)
