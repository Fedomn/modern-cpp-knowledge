# Add source to this project's executable.
add_executable (map_benchmark "map_benchmark.cpp")

# Link Google Benchmark to the project
target_link_libraries(map_benchmark benchmark::benchmark)
