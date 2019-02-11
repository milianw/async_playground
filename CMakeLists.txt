cmake_minimum_required(VERSION 3.0)
project(async)

find_package(Threads)
find_package(Qt5 COMPONENTS Core)
add_executable(async main.cpp)
target_link_libraries(async Threads::Threads Qt5::Core)
