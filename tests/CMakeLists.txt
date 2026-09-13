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
    test_engine_systems.cpp
    test_game_loop.cpp
    test_gameplay.cpp
    test_play_state.cpp
    test_render_passes.cpp
    test_snake_rules.cpp
    test_spatial_index.cpp
    test_tiled_loader.cpp
    test_vector_2d.cpp
    test_widgets.cpp
)

target_link_libraries(tests PRIVATE game_lib doctest::doctest)
de_enable_warnings(tests)
# doctest's TEST_CASE/SUBCASE expand __COUNTER__ into our own sources, where
# -Wpedantic flags it; the header being a system include does not help.
target_compile_options(tests PRIVATE -Wno-c2y-extensions)

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
# Behaviour lock for refactors: two sim runs per seed must be byte-identical
# and match tests/golden/. Seed 0 dies without eating; seed 4 eats a bat, so
# growth and the bat respawn are covered too.
set(DE_GOLDEN_SEEDS 0 4)
foreach(_seed IN LISTS DE_GOLDEN_SEEDS)
    add_test(NAME sim_golden_seed${_seed}
        COMMAND ${CMAKE_COMMAND}
            -DGAME=$<TARGET_FILE:game>
            -DSEED=${_seed}
            -DGOLDEN=${CMAKE_CURRENT_SOURCE_DIR}/golden/seed${_seed}.csv
            -DOUT_DIR=${CMAKE_CURRENT_BINARY_DIR}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/sim_golden.cmake
    )
    list(APPEND DE_SIM_TESTS sim_golden_seed${_seed})
endforeach()

set_tests_properties(smoke_menu smoke_level smoke_sim ${DE_SIM_TESTS} PROPERTIES
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;SDL_RENDER_DRIVER=software"
    TIMEOUT 60
)
