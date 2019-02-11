cmake_minimum_required(VERSION 3.0)
project(async)

find_package(Threads)
find_package(Qt5 COMPONENTS Core)
find_package(KAsync CONFIG)
set(CMAKE_AUTOMOC TRUE)

add_executable(kasync_kitchensink kasync_kitchensink.cpp)
target_link_libraries(kasync_kitchensink Threads::Threads Qt5::Core KAsync)
