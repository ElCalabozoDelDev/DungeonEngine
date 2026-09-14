# The Dungeon Slime game: snake, bat, scenes and widgets.
# Everything reusable lives in `engine`.
#
# Split into a library plus a thin executable so the tests can link the
# gameplay systems; main() is the only thing not in the library.

add_library(game_lib STATIC
    src/plugins/game_plugin.cpp
    src/play_state.cpp
    src/scene/in_game_scene.cpp
    src/scene/options_scene.cpp
    src/scene/title_scene.cpp
    src/sim/sim_plugin.cpp
    src/systems/bat_system.cpp
    src/systems/grayscale_fade_system.cpp
    src/systems/snake_system.cpp
    src/ui/bitmap_font.cpp
    src/ui/ui_layout.cpp
    src/ui/ui_skin.cpp
    src/widgets/hud_widget.cpp
    src/widgets/options_widget.cpp
    src/widgets/title_widget.cpp
)

target_include_directories(game_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(game_lib PUBLIC engine)
de_enable_warnings(game_lib)

add_executable(game src/main.cpp)
de_enable_warnings(game)

set_target_properties(game PROPERTIES
    OUTPUT_NAME ${PROJECT_NAME}
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Note: SDL2::SDL2main is deliberately NOT linked, because main.cpp defines
# SDL_MAIN_HANDLED and provides its own main() instead of SDL2main's
# WinMain -> SDL_main shim.
target_link_libraries(game PRIVATE game_lib)

de_copy_runtime_dlls(game)

add_custom_target(sync_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
    ${CMAKE_SOURCE_DIR}/assets
    ${CMAKE_BINARY_DIR}/assets
    COMMENT "Syncing assets"
)
add_dependencies(game sync_assets)
