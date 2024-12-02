# Add source to this project's executable.
add_executable (vector_benchmark "vector_benchmark.cpp")

# Link Google Benchmark to the project
target_link_libraries(vector_benchmark benchmark::benchmark)
