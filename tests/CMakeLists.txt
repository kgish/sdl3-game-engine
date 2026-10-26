include(Catch)

add_executable(engine_tests
    test_camera.cpp
    test_fixed_timestep.cpp
    test_handle_pool.cpp
    test_input.cpp
    test_shader_reflection.cpp
    test_thirdparty.cpp
)
# Tests may reach into engine internals (src/) as well as the public API.
target_include_directories(engine_tests PRIVATE ${PROJECT_SOURCE_DIR}/engine/src)
target_link_libraries(engine_tests PRIVATE engine::engine SDL3::SDL3 Catch2::Catch2WithMain)
engine_target_defaults(engine_tests)

set(sanitizer_env)
if(ENGINE_ENABLE_SANITIZERS)
    set(sanitizer_env
        "UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1"
        "LSAN_OPTIONS=suppressions=${PROJECT_SOURCE_DIR}/tools/sanitizers/lsan.supp"
    )
endif()

catch_discover_tests(engine_tests PROPERTIES ENVIRONMENT "${sanitizer_env}")

# Smoke test: open a window, render 120 frames, shut down cleanly.
# Needs a display and a GPU; exclude in headless CI with `ctest -LE gpu`.
if(ENGINE_BUILD_GAMES)
    add_test(NAME breakout_smoke COMMAND breakout)
    set_tests_properties(breakout_smoke PROPERTIES
        ENVIRONMENT "ENGINE_MAX_FRAMES=120;${sanitizer_env}"
        LABELS gpu
        TIMEOUT 30
    )
endif()
