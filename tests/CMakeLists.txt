# Unit tests. They link the engine and the game library directly, so they need
# no window, no input and no desktop session -- which is what lets them run in
# CI, and what made them the only way to verify several of the fixes here.

find_package(doctest CONFIG REQUIRED)

add_executable(tests
    main.cpp
    test_action_map.cpp
    test_animation.cpp
    test_balance.cpp
    test_config_loader.cpp
    test_gameplay.cpp
    test_play_state.cpp
    test_spatial_index.cpp
    test_tiled_loader.cpp
    test_vector_2d.cpp
)

target_link_libraries(tests PRIVATE game_lib doctest::doctest)

target_compile_definitions(tests PRIVATE
    DE_TEST_FIXTURES="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
    DE_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets"
)

set_target_properties(tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

de_copy_runtime_dlls(tests)

include(doctest)
doctest_discover_tests(tests)

add_test(NAME smoke_menu
    COMMAND game --frames 30
)
add_test(NAME smoke_level
    COMMAND game --level --frames 60
)

add_test(NAME smoke_sim
    COMMAND game --sim --sim-steps 120 --sim-out sim_smoke.csv
)
set_tests_properties(smoke_sim PROPERTIES
    PASS_REGULAR_EXPRESSION "sim: outcome="
)
set_tests_properties(smoke_menu smoke_level smoke_sim PROPERTIES
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;SDL_RENDER_DRIVER=software"
    TIMEOUT 60
)
