cmake_minimum_required(VERSION 3.21)
project(Bastionx VERSION 0.1.0 LANGUAGES CXX)

# C++20 requirement
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable Qt Meta-Object Compiler for signal/slot support
set(CMAKE_AUTOMOC ON)

# vcpkg integration
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

# Find dependencies
find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(unofficial-sodium CONFIG REQUIRED)
find_package(sqlcipher CONFIG REQUIRED)
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
    src/vault/VaultSettings.cpp
    src/storage/NotesRepository.cpp
)

target_include_directories(bastionx_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# SQLCipher requires SQLITE_HAS_CODEC for sqlite3_key/sqlite3_rekey declarations
target_compile_definitions(bastionx_core PUBLIC SQLITE_HAS_CODEC)

target_link_libraries(bastionx_core PUBLIC
    unofficial-sodium::sodium
    sqlcipher::sqlcipher
    nlohmann_json::nlohmann_json
)

# Executable: Bastionx GUI
add_executable(bastionx
    src/main.cpp
    src/ui/MainWindow.cpp
    src/ui/UnlockScreen.cpp
    src/ui/NotesPanel.cpp
    src/ui/NotesList.cpp
    src/ui/NoteEditor.cpp
    src/ui/ClipboardGuard.cpp
    src/ui/SettingsDialog.cpp
    # Headers with Q_OBJECT (needed for AUTOMOC)
    include/bastionx/ui/MainWindow.h
    include/bastionx/ui/UnlockScreen.h
    include/bastionx/ui/NotesPanel.h
    include/bastionx/ui/NotesList.h
    include/bastionx/ui/NoteEditor.h
    include/bastionx/ui/ClipboardGuard.h
    include/bastionx/ui/SettingsDialog.h
)
target_include_directories(bastionx PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(bastionx PRIVATE bastionx_core Qt6::Widgets)
if(WIN32)
    set_target_properties(bastionx PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# Install target
install(TARGETS bastionx RUNTIME DESTINATION bin)

# Tests
option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
