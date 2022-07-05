cmake_minimum_required (VERSION 3.20)

project (WesnothServer)

find_package (Boost 1.79.0 REQUIRED COMPONENTS iostreams)
find_package (spdlog 1.10.0 REQUIRED)

add_executable (${PROJECT_NAME}
	Source/Main.cpp
	Source/Server.hpp
	Source/Server.cpp
	Source/ClientHandler.hpp
	Source/ClientHandler.cpp
	Source/Gzip.hpp
	Source/Gzip.cpp
	Source/Versions.hpp
	Source/Config.hpp
	Source/Config.cpp
)

set_target_properties (${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 23
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

target_include_directories (${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries (${PROJECT_NAME} PRIVATE Boost::iostreams spdlog::spdlog)

# TODO: use COMPILE_WARNING_AS_ERROR instead of /WX flag once CMake 3.24 is available
if (MSVC)
	target_compile_options (${PROJECT_NAME} PRIVATE /W4 /WX)
endif ()
