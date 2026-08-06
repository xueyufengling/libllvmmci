cmake_minimum_required (VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use C export functions for ABI compability!
project(as)

set(PROJECT_PACKAGE_NAME ${PROJECT_NAME})

# Set zlib find path. zlib doesn't have Config.cmake so find_package() cannot find it.
set(CMAKE_PREFIX_PATH "C:/msys64/ucrt64")
find_package(LLVM REQUIRED)
find_package(ppmp REQUIRED)
find_package(arch REQUIRED)
find_package(sys-llvm REQUIRED)

# link_directories() must be called before target is created.
link_directories("${arch_LIBRARY_DIRS}")
link_directories("${sys_llvm_LIBRARY_DIRS}")

set(PROJECT_SOURCE_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include")

set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
file(GLOB_RECURSE SOURCES "${SOURCE_DIR}/*.cpp")

add_compile_options(-g -O0)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_INCLUDE_DIRS}")
target_include_directories(${PROJECT_NAME} PUBLIC "${ppmp_INCLUDE_DIRS}")
target_include_directories(${PROJECT_NAME} PUBLIC "${arch_INCLUDE_DIRS}")
target_include_directories(${PROJECT_NAME} PUBLIC "${sys_llvm_INCLUDE_DIRS}")

target_link_libraries(${PROJECT_NAME} "${sys_llvm_LIBRARIES}")

# Install
set(PROJECT_INSTALL_LIBRARIES ${PROJECT_NAME})
include("./cmake/install.cmake")