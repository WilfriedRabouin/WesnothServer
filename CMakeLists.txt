cmake_minimum_required (VERSION 3.20)

project (WesnothServer)

find_package (Boost 1.79.0 REQUIRED)

set (SPDLOG_NO_THREAD_ID ON)
set (SPDLOG_NO_ATOMIC_LEVELS ON)
add_subdirectory (spdlog)

add_subdirectory (zlib)

add_executable (${PROJECT_NAME}
	Source/Main.cpp
	Source/Server.hpp
	Source/Server.cpp
	Source/ClientHandler.hpp
	Source/ClientHandler.cpp
)

set_target_properties (${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

target_include_directories (${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries (${PROJECT_NAME} PRIVATE spdlog::spdlog zlibstatic)

if (MSVC)
	target_compile_options (${PROJECT_NAME} PRIVATE /W3 /WX)
endif ()
