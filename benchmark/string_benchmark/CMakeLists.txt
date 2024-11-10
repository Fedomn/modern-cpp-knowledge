# Add source to this project's executable.
add_executable (string_benchmark "string_benchmark.cpp")

# Link Google Benchmark to the project
target_link_libraries(string_benchmark benchmark::benchmark)
