set(sources
    src/vector.cpp
)

set(exe_sources
		${sources}
)

set(headers
    include/mcpp/vector.h
    include/mcpp/output_container.h
)

set(test_sources
  # app
  src/app/limiter.cpp
  src/app/lock_free_pool.cpp
  src/app/mysql_test.cpp
  # concurrency
  src/concurrency/atomic_test.cpp
  src/concurrency/future_test.cpp
  src/concurrency/lock_test.cpp
  # c
  src/c/c_lib_test.cpp
  src/c/c_test.cpp
  src/c/string_test.cpp
  # pointer
  src/pointer/raii_test.cpp
  src/pointer/reference_test.cpp
  # basic
  src/basic_test.cpp
  src/oop_test.cpp
  src/lambda_test.cpp
  src/vector_test.cpp
  src/containers_test.cpp
  src/templated_test.cpp
  src/class_test.cpp
  src/lifetime_test.cpp
)
