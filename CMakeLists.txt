cmake_minimum_required(VERSION 3.16)
project(DISPatch)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets Network)

add_executable(DISPatch
        main.cpp
        include/DISPatch/AppConfig.h
        include/DISPatch/Constants.h
        include/DISPatch/DisProtocol.h
        include/DISPatch/DisTypes.h
        include/DISPatch/MainWindow.h
        include/DISPatch/Theme.h
        src/AppConfig.cpp
        src/DisProtocol.cpp
        src/MainWindow.cpp
        src/Theme.cpp)
target_include_directories(DISPatch PRIVATE include)
target_link_libraries(DISPatch PRIVATE Qt5::Widgets Qt5::Network)

configure_file(etc/DISPatch_config.json DISPatch_config.json COPYONLY)
