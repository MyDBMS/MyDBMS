cmake_minimum_required(VERSION 3.0)
project(my_dbms)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(FS_SRC fs/Filesystem.cpp fs/Filesystem.h fs/File.cpp fs/File.h fs/BufferManager.cpp fs/BufferManager.h fs/BufferPage.cpp fs/BufferPage.h)

set(RS_SRC rs/RecordSystem.cpp rs/RecordSystem.h rs/RecordFile.cpp rs/RecordFile.h rs/RecordFileMeta.h rs/RecordPageHeader.h)

set(IS_SRC is/IndexSystem.cpp is/IndexSystem.h is/IndexFile.cpp is/IndexFile.h is/IndexFileMeta.h is/IndexPageHeader.h)

add_executable(main main.cpp ${FS_SRC} ${RS_SRC} ${IS_SRC})
add_executable(fs-test tests/fs-test.cpp ${FS_SRC})
add_executable(rs-test tests/rs-test.cpp ${FS_SRC} ${RS_SRC})
add_executable(is-test tests/is-test.cpp ${FS_SRC} ${RS_SRC} ${IS_SRC})

add_test(test-fs ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fs-test)
add_test(test-rs ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rs-test)
add_test(test-is ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/is-test)

enable_testing()
