cmake_minimum_required(VERSION 3.0)
project(my_dbms)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(FS_SRC fs/Filesystem.cpp fs/Filesystem.h fs/File.cpp fs/File.h fs/BufferManager.cpp fs/BufferManager.h fs/BufferPage.cpp fs/BufferPage.h)

add_executable(main main.cpp ${FS_SRC})
add_executable(fs-test tests/fs-test.cpp ${FS_SRC})

add_test(test-fs ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fs-test)

enable_testing()
