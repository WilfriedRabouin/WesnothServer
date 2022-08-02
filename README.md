# Wesnoth Server

Wesnoth Server is a server for the game [The Battle for Wesnoth](https://www.wesnoth.org/) compatible with Windows and Linux. It is developed with stability, security and performance in mind.

## Building for Linux

### Prerequisites

- Git
- [CMake](https://cmake.org/) 3.21 or newer
- Clang 14 or newer
- [Vcpkg](https://vcpkg.io/)

### Steps

```bash
git clone https://github.com/WilfriedRabouin/WesnothServer.git
cd WesnothServer
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=<Debug|Release> -DCMAKE_TOOLCHAIN_FILE=<path to vcpkg>/scripts/buildsystems/vcpkg.cmake 
cmake --build .
```

## Possible Improvements

- Anti-Cheat system
- Multiprocessing
