set(sources
    src/tmp.cpp
    src/vector.cpp
)

set(exe_sources
		src/main.cpp
		${sources}
)

set(headers
    include/modern-cpp-knowledge/tmp.hpp
    include/modern-cpp-knowledge/vector.h
)

set(test_sources
  src/tmp_test.cpp
  src/basic_test.cpp
  src/oop_test.cpp
  src/lambda_test.cpp
  src/vector_test.cpp
  src/containers_test.cpp
  src/raii_test.cpp
  src/concurrency_test.cpp
  src/reference_test.cpp
)
