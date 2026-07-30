cmake_minimum_required (VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use C export functions for ABI compability!
project(as)

# Set zlib find path. zlib doesn't have Config.cmake so find_package() cannot find it.
set(CMAKE_PREFIX_PATH "C:/msys64/ucrt64")
find_package(LLVM REQUIRED)
find_package(ppmp REQUIRED)
find_package(arch REQUIRED)

# link_directories() must be called before target is created.
link_directories("${arch_LIBRARY_DIRS}")

set(PROJECT_SOURCE_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include")

set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
file(GLOB_RECURSE SOURCES "${SOURCE_DIR}/*.cpp")

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_INCLUDE_DIRS}")
target_include_directories(${PROJECT_NAME} PUBLIC "${ppmp_INCLUDE_DIRS}")
target_include_directories(${PROJECT_NAME} PUBLIC "${arch_INCLUDE_DIRS}")

target_compile_definitions(${PROJECT_NAME} PUBLIC ${LLVM_DEFINITIONS})

# Set LLVM backend arch
# DO NOTE that not all backends are provided in msys2 package
# Backend arch name is case sensitive
set(LLVM_BACKENDS_ARCH X86)

# Link LLVM and its dependency
# Assembler and Disassembler
set(LLVM_LINK_COMPONENTS
	mc
	object
	support
	target)

llvm_map_components_to_libnames(llvm_libs ${LLVM_LINK_COMPONENTS} ${LLVM_BACKENDS_ARCH})

target_link_libraries(${PROJECT_NAME} ${llvm_libs} "${arch_LIBRARIES}")

message(STATUS "${arch_LIBRARY_DIRS}")

# Install
 
set(PROJECT_INSTALL_INCLUDE_DIRS "${CMAKE_INSTALL_PREFIX}/include")
set(PROJECT_INSTALL_LIBRARY_DIRS "${CMAKE_INSTALL_PREFIX}/lib")
set(PROJECT_INSTALL_LIBRARIES ${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME} DESTINATION "lib")
install(DIRECTORY "${PROJECT_SOURCE_INCLUDE_DIRS}/" DESTINATION "include")
message(STATUS "set copy path from \"${PROJECT_SOURCE_INCLUDE_DIRS}\" to \"${PROJECT_INSTALL_INCLUDE_DIRS}\"")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.cmake.in" "${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}Config.cmake" @ONLY)
message(STATUS "generated ${PROJECT_NAME}Config.cmake in: \"${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}Config.cmake\"")

message(STATUS "configure complete, run cmake --build . --target install to copy include directory")