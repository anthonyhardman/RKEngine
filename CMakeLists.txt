cmake_minimum_required(VERSION 3.24)

project(RKEngine VERSION 0.1)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED True)

include(FetchContent)

message(STATUS "Fetching GLFW")
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

find_package(Vulkan REQUIRED)
if(NOT Vulkan_FOUND)
  message(FATAL_ERROR "Vulkan not found")
endif()

add_library(RKEngine STATIC
  ${PROJECT_SOURCE_DIR}/src/renderer.cpp
  ${PROJECT_SOURCE_DIR}/src/renderer_factory.cpp
  ${PROJECT_SOURCE_DIR}/src/vulkan_renderer.cpp
  ${PROJECT_SOURCE_DIR}/src/vulkan_validation.cpp
)

target_include_directories(
  RKEngine 
  PUBLIC ${PROJECT_SOURCE_DIR}/include
  PUBLIC ${glfw_SOURCE_DIR}/include
  PUBLIC ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(
  RKEngine 
  PRIVATE glfw
  PRIVATE ${Vulkan_LIBRARIES}
)

add_subdirectory(examples)