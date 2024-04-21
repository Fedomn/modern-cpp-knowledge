set(sources
    src/tmp.cpp
    src/vector.cpp
)

set(exe_sources
		src/memoryAccess.cpp
		${sources}
)

set(headers
    include/mcpp/tmp.hpp
    include/mcpp/vector.h
    include/mcpp/output_container.h
)

set(test_sources
  src/lock_free_pool.cpp
  src/tmp_test.cpp
  src/basic_test.cpp
  src/oop_test.cpp
  src/lambda_test.cpp
  src/vector_test.cpp
  src/containers_test.cpp
  src/raii_test.cpp
  src/concurrency_test.cpp
  src/reference_test.cpp
  src/c_lib_test.cpp
  src/templated_test.cpp
  src/mysql_test.cpp
  src/string_test.cpp
  src/class_test.cpp
)
