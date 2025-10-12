cmake_minimum_required(VERSION 3.10)

project(MyHttpServer)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_executable(SimpleHttpServer
    main.cpp
)

if(UNIX)
    find_package(Threads REQUIRED)
    target_link_libraries(SimpleHttpServer pthread)
endif()