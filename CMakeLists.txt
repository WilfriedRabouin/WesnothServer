cmake_minimum_required (VERSION 3.12)

project (WesnothServer)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost 1.79.0 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

if (MSVC)
    add_compile_options (/W4 /WX)
endif ()

add_executable (${PROJECT_NAME}
	Source/Main.cpp
	Source/Server.hpp
	Source/Server.cpp
	Source/ClientHandler.hpp
	Source/ClientHandler.cpp)
