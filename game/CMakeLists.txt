# The example game: player input, enemies, items, the scenes and the widgets.
# Everything reusable lives in `engine`.
#
# Split into a library plus a thin executable so the tests can link the
# gameplay systems; main() is the only thing not in the library.

add_library(game_lib STATIC
    src/plugins/game_plugin.cpp
    src/scene/in_game_scene.cpp
    src/scene/menu_scene.cpp
    src/sim/nav_grid.cpp
    src/sim/seek_items_policy.cpp
    src/sim/sim_plugin.cpp
    src/sim/telemetry_sampler.cpp
    src/sim/telemetry_writer.cpp
    src/systems/combat_system.cpp
    src/systems/enemy_ai_system.cpp
    src/systems/movement_system.cpp
    src/widgets/counter_widget.cpp
    src/widgets/hud_widget.cpp
    src/widgets/menu_widget.cpp
)

target_include_directories(game_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(game_lib PUBLIC engine)

add_executable(game src/main.cpp)

# The target is `game`, but the binary keeps the project's name.
set_target_properties(game PROPERTIES
    OUTPUT_NAME ${PROJECT_NAME}
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Note: SDL2::SDL2main is deliberately NOT linked, because main.cpp defines
# SDL_MAIN_HANDLED and provides its own main() instead of SDL2main's
# WinMain -> SDL_main shim. Linking it forces the WinMain symbol, which in
# turn requires SDL_main and breaks the link step.
target_link_libraries(game PRIVATE game_lib)

# Every DLL the executable needs, next to it.
de_copy_runtime_dlls(game)

# Mirror assets next to the build.
#
# A target of its own, not a POST_BUILD on `game`: POST_BUILD runs only when
# the target is relinked, so editing a .tmx or a texture without touching any
# source left the old copy in build/assets and the change appeared to do
# nothing. A custom target is always considered out of date, and
# copy_directory_if_different only touches files whose contents changed, so
# this stays cheap.
add_custom_target(sync_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
    ${CMAKE_SOURCE_DIR}/assets
    ${CMAKE_BINARY_DIR}/assets
    COMMENT "Syncing assets"
)
add_dependencies(game sync_assets)
