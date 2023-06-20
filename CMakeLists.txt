cmake_minimum_required(VERSION 3.24)
project(labwork_12_notoriginallink LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(lib/CoolDB)

add_executable(main
        main.cpp)

target_link_libraries(main PRIVATE CoolDB)