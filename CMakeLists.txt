cmake_minimum_required(VERSION 2.8)
project(sqlite3_wrapper)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/uthash/include)

set(SQLITE_WRAPPER_SRC
        src/content.c
        src/cursor.c
        src/query_builder.c
        src/sqlite_wrapper.c)

add_library(sqlite_wrapper SHARED ${SQLITE_WRAPPER_SRC})
target_link_libraries(sqlite_wrapper PUBLIC sqlite3)

add_executable(demo example/example.c)
target_link_libraries(demo sqlite_wrapper)

