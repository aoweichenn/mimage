cmake_minimum_required(VERSION 3.24)
project(mimage_consumer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(mimage 0.1 CONFIG REQUIRED)

add_executable(mimage_consumer main.cpp)
target_link_libraries(mimage_consumer PRIVATE mimage::mimage)
