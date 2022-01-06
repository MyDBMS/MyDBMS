cmake_minimum_required(VERSION 3.0)
project(my_dbms)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(UTILS_SRC utils/error.h)

set(FS_SRC fs/Filesystem.cpp fs/Filesystem.h fs/File.cpp fs/File.h fs/BufferManager.cpp fs/BufferManager.h fs/BufferPage.cpp fs/BufferPage.h)

set(RS_SRC rs/RecordSystem.cpp rs/RecordSystem.h rs/RecordFile.cpp rs/RecordFile.h rs/RecordFileMeta.h rs/RecordPageHeader.h)

set(IS_SRC is/IndexSystem.cpp is/IndexSystem.h is/IndexFile.cpp is/IndexFile.h is/IndexFileMeta.h is/IndexPageHeader.h)

set(MS_SRC ms/ManageSystem.cpp ms/ManageSystem.h ms/DatabaseMapping.h ms/TableMapping.h ms/Field.h ms/Value.h ms/Value.cpp)

add_executable(main main.cpp ${UTILS_SRC} ${FS_SRC} ${RS_SRC} ${IS_SRC} ${MS_SRC})
add_executable(fs-test tests/fs-test.cpp ${UTILS_SRC} ${FS_SRC})
add_executable(rs-test tests/rs-test.cpp ${UTILS_SRC} ${FS_SRC} ${RS_SRC})
add_executable(is-test tests/is-test.cpp ${UTILS_SRC} ${FS_SRC} ${RS_SRC} ${IS_SRC})
add_executable(ms-test tests/ms-test.cpp ${UTILS_SRC} ${FS_SRC} ${RS_SRC} ${IS_SRC} ${MS_SRC})

add_test(test-fs ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fs-test)
add_test(test-rs ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rs-test)
add_test(test-is ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/is-test)
add_test(test-ms ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ms-test)

enable_testing()
