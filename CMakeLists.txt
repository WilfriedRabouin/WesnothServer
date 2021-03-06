﻿cmake_minimum_required (VERSION 3.21)

project (WesnothServer)

find_package (Boost REQUIRED COMPONENTS iostreams program_options)
find_package (ZLIB REQUIRED) # necessary for Boost::iostreams
find_package (spdlog REQUIRED)
find_package (fmt REQUIRED)

add_executable (${PROJECT_NAME}
	Source/Main.cpp
	Source/Server.hpp
	Source/Server.cpp
	Source/ClientHandler.hpp
	Source/ClientHandler.cpp
	Source/Gzip.hpp
	Source/Gzip.cpp
	Source/Versions.hpp
	Source/ProgramOptions.hpp
	Source/ProgramOptions.cpp
	Source/PoolAllocator.hpp
)

set_target_properties (${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 23
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

target_include_directories (${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries (${PROJECT_NAME} PRIVATE Boost::iostreams Boost::program_options)
target_link_libraries (${PROJECT_NAME} PRIVATE ZLIB::ZLIB) # necessary for Boost::iostreams
target_link_libraries (${PROJECT_NAME} PRIVATE spdlog::spdlog)
target_link_libraries (${PROJECT_NAME} PRIVATE fmt::fmt)

# TODO: use COMPILE_WARNING_AS_ERROR instead of /WX and -Werror flags once CMake 3.24 is available for MSVC and Fedora Server
if (MSVC)
	target_compile_options (${PROJECT_NAME} PRIVATE /W4 /WX)
else ()
	target_compile_options (${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror)
endif ()

add_custom_command (
	TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/Source/Banner.txt ${CMAKE_CURRENT_BINARY_DIR}/Banner.txt
)
