# Wesnoth Server

Wesnoth Server is a server for the game [The Battle for Wesnoth](https://www.wesnoth.org/) compatible with Windows and Linux. It is developed with stability, security and performance in mind.

## Building

### Prerequisites

- [Git](https://git-scm.com/)
- [CMake](https://cmake.org/) 3.21 or newer
- A C++23 compiler ([Clang](https://clang.llvm.org/) 14, [Visual C++](https://visualstudio.microsoft.com) 2022 or newer)
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
