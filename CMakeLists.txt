cmake_minimum_required(VERSION 3.0)
project(async)

find_package(Threads)
find_package(Qt5 COMPONENTS Core)
find_package(KAsync CONFIG)
set(CMAKE_AUTOMOC TRUE)

add_executable(kasync_kitchensink kasync_kitchensink.cpp)
target_link_libraries(kasync_kitchensink Threads::Threads Qt5::Core KAsync)

add_executable(qfuture_kitchensink qfuture_kitchensink.cpp)
target_link_libraries(qfuture_kitchensink Threads::Threads Qt5::Core)

include_directories(qtpromise/include)
add_executable(qpromise_kitchensink qpromise_kitchensink.cpp)
target_link_libraries(qpromise_kitchensink Threads::Threads Qt5::Core)

include_directories(asyncfuture)
add_executable(asyncfuture_kitchensink asyncfuture_kitchensink.cpp)
target_link_libraries(asyncfuture_kitchensink Threads::Threads Qt5::Core)
