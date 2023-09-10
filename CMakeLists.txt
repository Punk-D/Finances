cmake_minimum_required(VERSION 3.27.4 FATAL_ERROR)

project(Finances-CMake-git)

add_executable(Finances src/main.cpp)

include_directories(${src})

include(FetchContent)

FetchContent_Declare(
    sqlite3
    GIT_REPOSITORY https://github.com/sqlite/sqlite.git
    GIT_TAG version-3.43.0  # Replace with the desired SQLite version/tag
)

FetchContent_MakeAvailable(sqlite3)

target_link_libraries(Finances PRIVATE SQLite::SQLite3)


