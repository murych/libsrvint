
add_executable(example)
target_sources(example PRIVATE main.c)
target_link_libraries(example PRIVATE libsrvint)
target_compile_features(example PRIVATE c_std_11)

enable_language(CXX)
add_executable(server_lambda_example server_lambda.cpp)
target_link_libraries(server_lambda_example PRIVATE libsrvint)
target_compile_features(server_lambda_example PRIVATE cxx_std_23)

add_executable(example_client example_client.cpp)
target_link_libraries(example_client PRIVATE libsrvint)
target_compile_features(example_client PRIVATE cxx_std_23)
