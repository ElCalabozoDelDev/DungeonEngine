# The reusable half of the boilerplate: game loop, plugin/hook system, SDL and
# Dear ImGui integration, texture and tilemap loading, rendering, camera,
# spatial index, scenes and the widget layer.
#
# Nothing here may reference the game. There is no game/include on this
# target's include path, which is what enforces that.

add_library(engine STATIC
    src/audio/audio_manager.cpp
    src/core/asset_paths.cpp
    src/input/action_map.cpp
    src/graphics/camera2d.cpp
    src/graphics/render_bottom.cpp
    src/graphics/render_collision.cpp
    src/graphics/render_object.cpp
    src/graphics/render_overlay.cpp
    src/graphics/texture_cache.cpp
    src/graphics/world_color_grade_pass.cpp
    src/loaders/config_loader.cpp
    src/loaders/tiled_loader.cpp
    src/plugins/audio_plugin.cpp
    src/plugins/base_plugin.cpp
    src/plugins/imgui_plugin.cpp
    src/plugins/input_plugin.cpp
    src/plugins/sdl_plugin.cpp
    src/scene/scene_system.cpp
    src/spatial/spatial_index.cpp
    src/systems/camera_system.cpp
    src/systems/collision_system.cpp
    src/systems/debug_system.cpp
    src/systems/render_system.cpp
    src/systems/spatial_sync_system.cpp
    src/systems/transform_system.cpp
    src/systems/update_animation_system.cpp
)

target_include_directories(engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

de_enable_warnings(engine)

# PUBLIC: these appear in engine's public headers (SDL types, entt::registry,
# nlohmann::json in tiled_loader, the imgui backends via debug.hpp).
target_link_libraries(engine PUBLIC
    third_party
    SDL2::SDL2
    SDL2_image::SDL2_image
    SDL2_mixer::SDL2_mixer
    EnTT::EnTT
    imgui::imgui
    nlohmann_json::nlohmann_json
)
