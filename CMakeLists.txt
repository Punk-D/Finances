cmake_minimum_required(VERSION 3.12)
project(Finances)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)

# Specify relative paths to SQLite3 and OpenSSL directories
set(SQLite3_DIR "${CMAKE_SOURCE_DIR}/external_libs/SQLite3")
set(OpenSSL_ROOT_DIR "${CMAKE_SOURCE_DIR}/external_libs/openssl")

# Find SQLite3 package
find_package(SQLite3 REQUIRED)

# Find OpenSSL package
find_package(OpenSSL REQUIRED)

# Add your source files
set(SOURCE_FILES
    main.cpp
    SQLite_functions.cpp
    # Add other .cpp files here
)

# Add executable target
add_executable(Finances ${SOURCE_FILES})

# Link the SQLite3 library
target_link_libraries(Finances SQLite::SQLite3)

# Link the OpenSSL library
target_link_libraries(Finances OpenSSL::Crypto OpenSSL::SSL)
