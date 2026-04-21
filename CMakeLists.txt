cmake_minimum_required (VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use C export functions for ABI compability!
project(llvmmci)

# Set zlib find path. zlib doesn't have Config.cmake so find_package() cannot find it.
set(CMAKE_PREFIX_PATH "C:/msys64/ucrt64")
find_package(LLVM REQUIRED)

set(PROJECT_SOURCE_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include")

set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
file(GLOB_RECURSE SOURCES "${SOURCE_DIR}/*.cpp")

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC "${LLVM_INCLUDE_DIRS}")
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_INCLUDE_DIRS}")

target_compile_definitions(${PROJECT_NAME} PUBLIC ${LLVM_DEFINITIONS})

# Set LLVM backend arch
# DO NOTE that not all backends are provided in msys2 package
# Backend arch name is case sensitive
set(LLVM_BACKENDS_ARCH X86)

# Link LLVM and its dependency
set(LLVM_LINK_COMPONENTS
	# Runtime linker
	orcjit
	executionengine
	# Assembler and Disassembler
	mc
	mcparser
	support 
	target)

llvm_map_components_to_libnames(llvm_libs ${LLVM_LINK_COMPONENTS} ${LLVM_BACKENDS_ARCH})

target_link_libraries(${PROJECT_NAME} ${llvm_libs})

# Install
 
set(PROJECT_INSTALL_INCLUDE_DIRS "${CMAKE_INSTALL_PREFIX}/include")
set(PROJECT_INSTALL_LIBRARIES "${CMAKE_INSTALL_PREFIX}/lib")

install(TARGETS ${PROJECT_NAME} DESTINATION "lib")
install(DIRECTORY "${PROJECT_SOURCE_INCLUDE_DIRS}/" DESTINATION "include")
message(STATUS "set copy path from \"${PROJECT_SOURCE_INCLUDE_DIRS}\" to \"${PROJECT_INSTALL_INCLUDE_DIRS}\"")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.cmake.in" "${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}Config.cmake" @ONLY)
message(STATUS "generated ${PROJECT_NAME}Config.cmake in: \"${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}Config.cmake\"")

message(STATUS "configure complete, run cmake --build . --target install to copy include directory")