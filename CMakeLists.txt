cmake_minimum_required(VERSION 3.10)

project(RKEngine VERSION 0.1)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED True)

include(FetchContent)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.3.8
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(glfw)


set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_library(RKEngine STATIC
  ${PROJECT_SOURCE_DIR}/src/renderer.cpp
)

target_include_directories(
  RKEngine 
  PUBLIC ${PROJECT_SOURCE_DIR}/include
  PUBLIC ${glfw_SOURCE_DIR}/include
)

target_link_libraries(
  RKEngine 
  PRIVATE glfw
)

add_subdirectory(examples)

