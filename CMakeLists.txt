cmake_minimum_required(VERSION 3.20)
project(my_dbms)

include_directories(
        ${PROJECT_SOURCE_DIR}/generated/
        ${PROJECT_SOURCE_DIR}/antlr_runtime/
        ${PROJECT_SOURCE_DIR}/fs/
        ${PROJECT_SOURCE_DIR}/rs/
        ${PROJECT_SOURCE_DIR}/is/
        ${PROJECT_SOURCE_DIR}/qs/
        ${PROJECT_SOURCE_DIR}/ms/
        ${PROJECT_SOURCE_DIR}/src/
)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "-std=c++20")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(UTILS_SRC utils/Error.h utils/Frontend.cpp utils/Frontend.h utils/Application.cpp utils/Application.h)

set(FS_SRC fs/Filesystem.cpp fs/Filesystem.h fs/File.cpp fs/File.h fs/BufferManager.cpp fs/BufferManager.h fs/BufferPage.cpp fs/BufferPage.h)

set(RS_SRC rs/RecordSystem.cpp rs/RecordSystem.h rs/RecordFile.cpp rs/RecordFile.h rs/RecordFileMeta.h rs/RecordPageHeader.h)

set(IS_SRC is/IndexSystem.cpp is/IndexSystem.h is/IndexFile.cpp is/IndexFile.h is/IndexFileMeta.h is/IndexPageHeader.h)

set(QS_SRC qs/QuerySystem.cpp qs/QuerySystem.h qs/SelectStmt.h qs/SelectStmt.cpp qs/WhereClause.h qs/WhereClause.cpp qs/SelectStmt.h qs/Selector.h qs/Column.h qs/Column.cpp qs/RecordSet.h qs/RecordSet.cpp)

set(MS_SRC ms/ManageSystem.cpp ms/ManageSystem.h ms/DatabaseMapping.h ms/TableMapping.h ms/Field.h ms/Value.h ms/Value.cpp)

set(GR_SRC
        generated/SQLLexer.cpp
        generated/SQLLexer.h
        generated/SQLParser.cpp
        generated/SQLParser.h
        generated/SQLVisitor.cpp
        generated/SQLVisitor.h
        generated/SQLBaseVisitor.cpp
        generated/SQLBaseVisitor.h
)

file(GLOB antlr4-cpp-src
    ${PROJECT_SOURCE_DIR}/antlr_runtime/*.cpp
    ${PROJECT_SOURCE_DIR}/antlr_runtime/atn/*.cpp
    ${PROJECT_SOURCE_DIR}/antlr_runtime/dfa/*.cpp
    ${PROJECT_SOURCE_DIR}/antlr_runtime/misc/*.cpp
    ${PROJECT_SOURCE_DIR}/antlr_runtime/support/*.cpp
    ${PROJECT_SOURCE_DIR}/antlr_runtime/tree/*.cpp
    ${PROJECT_SOURCE_DIR}/antlr_runtime/tree/pattern/*.cpp
    ${PROJECT_SOURCE_DIR}/antlr_runtime/tree/xpath/*.cpp
)

add_library (antlr4-cpp-runtime ${antlr4-cpp-src})

add_executable(my_dbms ${UTILS_SRC} ${FS_SRC} ${RS_SRC} ${IS_SRC} ${QS_SRC} ${MS_SRC} ${GR_SRC} src/main.cpp)
target_link_libraries(my_dbms antlr4-cpp-runtime)

add_executable(utils-test tests/utils-test.cpp ${UTILS_SRC} ${FS_SRC} ${RS_SRC} ${IS_SRC} ${QS_SRC} ${MS_SRC} ${GR_SRC})
add_executable(fs-test tests/fs-test.cpp ${FS_SRC})
add_executable(rs-test tests/rs-test.cpp ${FS_SRC} ${RS_SRC})
add_executable(is-test tests/is-test.cpp ${FS_SRC} ${RS_SRC} ${IS_SRC})
add_executable(ms-test tests/ms-test.cpp ${UTILS_SRC} ${FS_SRC} ${RS_SRC} ${IS_SRC} ${QS_SRC} ${MS_SRC} ${GR_SRC})
target_link_libraries(utils-test antlr4-cpp-runtime)
target_link_libraries(ms-test antlr4-cpp-runtime)

add_test(test-utils ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/utils-test)
add_test(test-fs ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fs-test)
add_test(test-rs ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rs-test)
add_test(test-is ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/is-test)
add_test(test-ms ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ms-test)

enable_testing()
