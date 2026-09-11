# The example game: player input, enemies, items, the scenes and the widgets.
# Everything reusable lives in `engine`.
#
# Split into a library plus a thin executable so the tests can link the
# gameplay systems; main() is the only thing not in the library.

add_library(game_lib STATIC
    src/plugins/game_plugin.cpp
    src/scene/in_game_scene.cpp
    src/scene/menu_scene.cpp
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

# Copy SDL2.dll and the other required runtime DLLs next to the executable.
add_custom_command(TARGET game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_RUNTIME_DLLS:game>
    $<TARGET_FILE_DIR:game>
    COMMAND_EXPAND_LISTS
)

# Mirror assets next to the build. copy_directory_if_different only touches
# files whose contents changed, so this is cheap on repeat builds and does
# not wipe the destination the way remove_directory + copy_directory did.
add_custom_command(TARGET game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
    ${CMAKE_SOURCE_DIR}/assets
    ${CMAKE_BINARY_DIR}/assets
)
