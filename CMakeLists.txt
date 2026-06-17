cmake_minimum_required(VERSION 3.16)
project(DISPatch)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets Network)

add_executable(DISPatch main.cpp)
target_link_libraries(DISPatch PRIVATE Qt5::Widgets Qt5::Network)
