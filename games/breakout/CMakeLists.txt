add_executable(breakout src/main.cpp)
target_link_libraries(breakout PRIVATE engine::engine)
engine_target_defaults(breakout)
