cmake_minimum_required(VERSION 3.10)

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

message(STATUS "Checking for Vulkan SDK")
if (NOT DEFINED ENV{VULKAN_SDK})
  message(FATAL_ERROR "VULKAN_SDK environment variable is not set")
endif()
message(STATUS "VULKAN SDK found at $ENV{VULKAN_SDK}")

message(STATUS "Getting Vulkan lib path")
file(TO_CMAKE_PATH "$ENV{VK_SDK_PATH}/Lib/vulkan-1.lib" VK_LIB_PATH)
message(STATUS "Vulkan lib path is ${VK_LIB_PATH}")

add_library(RKEngine STATIC
  ${PROJECT_SOURCE_DIR}/src/renderer.cpp
  ${PROJECT_SOURCE_DIR}/src/renderer_factory.cpp
  ${PROJECT_SOURCE_DIR}/src/vulkan_renderer.cpp
)

target_include_directories(
  RKEngine 
  PUBLIC ${PROJECT_SOURCE_DIR}/include
  PUBLIC ${glfw_SOURCE_DIR}/include
  PUBLIC $ENV{VK_SDK_PATH}/Include
)

target_link_libraries(
  RKEngine 
  PRIVATE glfw
  PRIVATE ${VK_LIB_PATH}
)

add_subdirectory(examples)