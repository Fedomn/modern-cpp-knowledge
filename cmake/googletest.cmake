add_subdirectory(${CMAKE_SOURCE_DIR}/deps/googletest)

SET(GMOCK_SOURCE_DIR
  ${CMAKE_SOURCE_DIR}/deps/googletest/googlemock)
SET(GTEST_SOURCE_DIR
  ${CMAKE_SOURCE_DIR}/deps/googletest/googletest)
SET(G_INCLUDE_DIRS
  ${GMOCK_SOURCE_DIR}
  ${GMOCK_SOURCE_DIR}/include
  ${GTEST_SOURCE_DIR}
  ${GTEST_SOURCE_DIR}/include
  CACHE INTERNAL "")
INCLUDE_DIRECTORIES(${G_INCLUDE_DIRS})
