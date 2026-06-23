
add_executable(example)
target_sources(example PRIVATE main.c)
target_link_libraries(example PRIVATE libsrvint)
target_compile_features(example PRIVATE c_std_11)
