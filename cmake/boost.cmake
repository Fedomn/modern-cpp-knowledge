# see boost/tools/cmake/README.md
# https://github.com/boostorg/cmake/tree/3a5c500a490ec25c502b912bc0357249b0816fd0

set(BOOST_INCLUDE_LIBRARIES timer lockfree)
add_subdirectory(${CMAKE_SOURCE_DIR}/deps/boost EXCLUDE_FROM_ALL)
