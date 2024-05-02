# https://github.com/apache/brpc/blob/master/docs/cn/getting_started.md
# deps/brpc/example/grpc_c++/CMakeLists.txt

# disable compile warning: 
# add the following line after `add_libray(brpc-static)` in `deps/brpc/src/CMakeLists.txt`
# target_compile_options(brpc-static PUBLIC "-w")

add_subdirectory(${CMAKE_SOURCE_DIR}/deps/brpc)
include_directories(${CMAKE_SOURCE_DIR}/deps/brpc/src ${CMAKE_SOURCE_DIR}/deps/brpc/output/include)

find_path(GFLAGS_INCLUDE_PATH gflags/gflags.h)
find_library(GFLAGS_LIBRARY NAMES gflags libgflags)
if((NOT GFLAGS_INCLUDE_PATH) OR (NOT GFLAGS_LIBRARY))
    message(FATAL_ERROR "Fail to find gflags")
endif()
include_directories(${GFLAGS_INCLUDE_PATH})

find_package(OpenSSL)
include_directories(${OPENSSL_INCLUDE_DIR})

find_path(LEVELDB_INCLUDE_PATH NAMES leveldb/db.h)
find_library(LEVELDB_LIB NAMES leveldb)
if ((NOT LEVELDB_INCLUDE_PATH) OR (NOT LEVELDB_LIB))
    message(FATAL_ERROR "Fail to find leveldb")
endif()
include_directories(${LEVELDB_INCLUDE_PATH})

find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)

set(DYNAMIC_LIB
    ${CMAKE_THREAD_LIBS_INIT}
    ${GFLAGS_LIBRARY}
    ${PROTOBUF_LIBRARIES}
    ${LEVELDB_LIB}
    ${OPENSSL_CRYPTO_LIBRARY}
    ${OPENSSL_SSL_LIBRARY}
    dl
    )
