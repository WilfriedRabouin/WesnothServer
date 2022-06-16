cmake_minimum_required (VERSION 3.12)

project (WesnothServer)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

find_package (Boost 1.79.0 REQUIRED)

set (SPDLOG_NO_THREAD_ID ON)
set (SPDLOG_NO_ATOMIC_LEVELS ON)
add_subdirectory (spdlog)

add_executable (${PROJECT_NAME}
	Source/Main.cpp
	Source/Server.hpp
	Source/Server.cpp
	Source/ClientHandler.hpp
	Source/ClientHandler.cpp
)

target_include_directories (${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries (${PROJECT_NAME} PRIVATE spdlog::spdlog)

if (MSVC)
	# TO FIX:
	#target_compile_options (${PROJECT_NAME} PRIVATE /Wall /WX)
endif ()
