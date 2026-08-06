set(PROJECT_INSTALL_INCLUDE_DIRS "${CMAKE_INSTALL_PREFIX}/include")
set(PROJECT_INSTALL_LIBRARY_DIRS "${CMAKE_INSTALL_PREFIX}/lib")

install(TARGETS ${PROJECT_INSTALL_LIBRARIES} DESTINATION "lib")
install(DIRECTORY "${PROJECT_SOURCE_INCLUDE_DIRS}/" DESTINATION "include")
message(STATUS "set copy path from \"${PROJECT_SOURCE_INCLUDE_DIRS}\" to \"${PROJECT_INSTALL_INCLUDE_DIRS}\"")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.cmake.in" "${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}Config.cmake" @ONLY)
message(STATUS "generated ${PROJECT_NAME}Config.cmake in: \"${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}Config.cmake\"")

message(STATUS "configure complete, run cmake --build . --target install to copy include directory")