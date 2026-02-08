cmake_minimum_required(VERSION 3.21)
project(Bastionx VERSION 0.1.0 LANGUAGES CXX)

# C++20 requirement
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# vcpkg integration
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

# Find dependencies
find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(unofficial-sodium CONFIG REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4)
    # TODO: Enable /WX (warnings as errors) after initial development
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    # TODO: Enable -Werror after initial development
endif()

# Library: Bastionx Core (no UI)
add_library(bastionx_core STATIC
    src/crypto/CryptoService.cpp
    src/crypto/SecureMemory.cpp
    src/vault/VaultService.cpp
    src/storage/NotesRepository.cpp
)

target_include_directories(bastionx_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(bastionx_core PUBLIC
    unofficial-sodium::sodium
    SQLite::SQLite3
    nlohmann_json::nlohmann_json
)

# Executable (future - Phase 3)
# add_executable(bastionx src/main.cpp)
# target_link_libraries(bastionx PRIVATE bastionx_core Qt6::Widgets)

# Tests
option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
