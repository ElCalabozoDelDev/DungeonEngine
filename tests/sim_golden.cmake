# Runs `--sim` twice with the same seed and checks that
#   1. both runs wrote byte-identical CSVs (the sim is deterministic), and
#   2. they match the committed reference in tests/golden/.
#
# The reference is the acceptance test for refactors: a change that is meant
# to preserve behaviour must not move a single byte. A change that is meant to
# alter gameplay regenerates it on purpose, in the same commit:
#
#   cmake -DGAME=build/bin/DungeonEngine.exe -DSEED=<n> -DOUT_DIR=build
#         -DGOLDEN=tests/golden/seed<n>.csv -DUPDATE=ON
#         -P tests/sim_golden.cmake
#
# Invoked by CTest with -DGAME, -DSEED, -DGOLDEN and -DOUT_DIR.

foreach(_var GAME SEED GOLDEN OUT_DIR)
    if(NOT DEFINED ${_var})
        message(FATAL_ERROR "sim_golden.cmake: ${_var} is not set")
    endif()
endforeach()

set(_first "${OUT_DIR}/golden_seed${SEED}_a.csv")
set(_second "${OUT_DIR}/golden_seed${SEED}_b.csv")

foreach(_out IN ITEMS "${_first}" "${_second}")
    execute_process(
        COMMAND "${GAME}" --sim --sim-seed ${SEED} --sim-out "${_out}"
        RESULT_VARIABLE _result
        OUTPUT_VARIABLE _stdout
        ERROR_VARIABLE _stderr
    )
    if(NOT _result EQUAL 0)
        message(FATAL_ERROR
            "sim run failed (exit ${_result}):\n${_stdout}${_stderr}")
    endif()
endforeach()

# Reports the first line where two CSVs disagree, which is the step at which
# the behaviour diverged.
function(_first_difference left right out_var)
    file(STRINGS "${left}" _left)
    file(STRINGS "${right}" _right)
    list(LENGTH _left _left_count)
    list(LENGTH _right _right_count)
    set(_count ${_left_count})
    if(_right_count LESS _count)
        set(_count ${_right_count})
    endif()
    set(_index 0)
    while(_index LESS _count)
        list(GET _left ${_index} _l)
        list(GET _right ${_index} _r)
        if(NOT _l STREQUAL _r)
            math(EXPR _line "${_index} + 1")
            set(${out_var}
                "line ${_line}:\n  expected: ${_r}\n  actual:   ${_l}"
                PARENT_SCOPE)
            return()
        endif()
        math(EXPR _index "${_index} + 1")
    endwhile()
    set(${out_var}
        "files agree for ${_count} lines; expected ${_right_count} lines, got ${_left_count}"
        PARENT_SCOPE)
endfunction()

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E compare_files "${_first}" "${_second}"
    RESULT_VARIABLE _same_runs
)
if(NOT _same_runs EQUAL 0)
    _first_difference("${_second}" "${_first}" _where)
    message(FATAL_ERROR
        "sim is not deterministic: two runs with seed ${SEED} differ at ${_where}")
endif()

if(UPDATE)
    file(COPY_FILE "${_first}" "${GOLDEN}")
    message(STATUS "Updated ${GOLDEN}")
    return()
endif()

if(NOT EXISTS "${GOLDEN}")
    message(FATAL_ERROR "no reference CSV at ${GOLDEN}; generate it with -DUPDATE=ON")
endif()

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E compare_files "${_first}" "${GOLDEN}"
    RESULT_VARIABLE _matches_golden
)
if(NOT _matches_golden EQUAL 0)
    _first_difference("${_first}" "${GOLDEN}" _where)
    message(FATAL_ERROR
        "sim with seed ${SEED} no longer matches ${GOLDEN} at ${_where}\n"
        "If the gameplay change is intended, regenerate the reference "
        "(see the header of tests/sim_golden.cmake).")
endif()
