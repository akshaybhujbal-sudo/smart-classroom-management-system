cmake_minimum_required(VERSION 3.16)

# ==============================================================================
# Smart Classroom Management System — College Mini-Project Build Configuration
# Backend Language: Modern C++ (C++17)
# Architecture: Layered Modular C++ Backend with Embedded REST Web Server
# ==============================================================================

project(SmartClassroomManagementSystem
    VERSION 1.0.0
    DESCRIPTION "Smart Classroom Management System with C++17 Backend and REST API"
    LANGUAGES CXX
)

# Enforce strict modern C++17 standard compliance
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler Warning Flags for Clean, Safe Code
if(MSVC)
    add_compile_options(/W4 /EHsc /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    # Enable static linking for MinGW/GCC/Clang so executables run anywhere without missing DLLs
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
endif()

# Global Include Directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/third_party
)

# ------------------------------------------------------------------------------
# Core Modular Source Files
# ------------------------------------------------------------------------------
set(CORE_SOURCES
    src/storage/FileStorageManager.cpp
    src/services/StudentManager.cpp
    src/services/AttendanceManager.cpp
    src/services/TimetableManager.cpp
    src/services/ClassroomManager.cpp
    src/services/ReportService.cpp
    src/controllers/ApiController.cpp
)

# ------------------------------------------------------------------------------
# Application Executable Target
# ------------------------------------------------------------------------------
add_executable(smart_classroom_server
    src/main.cpp
    ${CORE_SOURCES}
)

# Windows Sockets & Networking Libraries
if(WIN32)
    target_link_libraries(smart_classroom_server PRIVATE ws2_32 wsock32 crypt32)
endif()

# ------------------------------------------------------------------------------
# Automated Test Suite Target
# ------------------------------------------------------------------------------
add_executable(smart_classroom_tests
    tests/test_backend.cpp
    ${CORE_SOURCES}
)

if(WIN32)
    target_link_libraries(smart_classroom_tests PRIVATE ws2_32 wsock32 crypt32)
endif()
