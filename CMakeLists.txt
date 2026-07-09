cmake_minimum_required(VERSION 3.14)
project(LibrarySystem CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ========== vcpkg ==========
set(CMAKE_TOOLCHAIN_FILE D:/work/vcpkg/scripts/buildsystems/vcpkg.cmake)

# ========== MSVC: force UTF-8 + fix std::byte conflict with Windows headers ==========
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
add_compile_definitions("$<$<CXX_COMPILER_ID:MSVC>:NOMINMAX>")
add_compile_definitions("$<$<CXX_COMPILER_ID:MSVC>:_HAS_STD_BYTE=0>")

# ========== Dependencies (via vcpkg) ==========
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(Drogon CONFIG REQUIRED)

# ========== Source files ==========
set(SOURCES
    main.cpp
    LibrarySystem.cpp
    controllers/ApiController.cpp
)

# ========== Executable ==========
add_executable(LibrarySystem ${SOURCES})

# ========== Link libraries ==========
target_link_libraries(LibrarySystem PRIVATE
    unofficial::sqlite3::sqlite3
    Drogon::Drogon
)
