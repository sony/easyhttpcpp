cmake_minimum_required(VERSION 3.0)

project(easyhttpcpp-samples)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#############################################################
# build sample targets

find_package(OpenSSL REQUIRED)

add_subdirectory(SimpleHttpClient)
add_subdirectory(AsyncHttpClient)
