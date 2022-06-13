cmake_minimum_required (VERSION 3.8)

project (WesnothServer)

set (CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    add_compile_options (/W4 /WX)
endif ()

add_executable (${PROJECT_NAME}
	Source/Main.cpp
	Source/Server.hpp
	Source/Server.cpp)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  #set_property (TARGET WesnothServer PROPERTY CXX_STANDARD 20)
endif ()
