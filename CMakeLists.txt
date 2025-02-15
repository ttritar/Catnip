#cmake_minimum_required(VERSION 3.27)
#
## Project Name
#project(GP2_VulkanTutorial)
#
## Use C++20
#set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#
## Find Vulkan
#find_package(Vulkan REQUIRED)
#
## Enable FetchContent module
#include(FetchContent)
#
## Fetch glfw
#FetchContent_Declare(
#	GLFW
#	GIT_REPOSITORY https://github.com/glfw/glfw.git
#	GIT_TAG 3.3.8
#)
## Make glfw available
#FetchContent_MakeAvailable(GLFW) 
#
## Fetch glm
#FetchContent_Declare(
#	glm
#	GIT_REPOSITORY https://github.com/g-truc/glm.git
#	GIT_TAG 0.9.9.8
#)
## Make glm available
#FetchContent_MakeAvailable(glm) 
#
#
## Define your target (e.g., an executable)
#add_executable(GP2_VulkanTutorial main.cpp)
#
## Link the fetched libraries to your target
#target_link_libraries(GP2_VulkanTutorial PRIVATE glfw glm Vulkan::Vulkan)
#
#target_include_directories(GP2_VulkanTutorial PRIVATE 
#    ${Vulkan_INCLUDE_DIRS}  # Vulkan headers
#    ${GLFW_SOURCE_DIR}/include  # GLFW headers
#    ${glm_SOURCE_DIR}  # GLM headers
#)
#
#target_link_directories(GP2_VulkanTutorial PRIVATE _deps/glfw-build/src)
#
#
#add_custom_command(TARGET GP2_VulkanTutorial POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        $<TARGET_FILE:glfw>
#        $<TARGET_FILE_DIR:GP2_VulkanTutorial>
#)
#
#
#message(STATUS "Vulkan Include Directories: ${Vulkan_INCLUDE_DIRS}")
#message(STATUS "GLFW Include Directories: ${GLFW_SOURCE_DIR}/include")
#message(STATUS "GLM Include Directories: ${glm_SOURCE_DIR}")

cmake_minimum_required(VERSION 3.27)

# Project Name
project(GP2_VulkanTutorial)

# Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Vulkan
find_package(Vulkan REQUIRED)

# Enable FetchContent module
include(FetchContent)

# Fetch GLFW
FetchContent_Declare(
    GLFW
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)
FetchContent_MakeAvailable(GLFW) 

# Fetch GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0.9.9.8
)
FetchContent_MakeAvailable(glm)

# Define the executable
add_executable(GP2_VulkanTutorial main.cpp)

# Link necessary libraries
target_link_libraries(GP2_VulkanTutorial PRIVATE glfw glm Vulkan::Vulkan)

# Include directories
target_include_directories(GP2_VulkanTutorial PRIVATE 
    ${Vulkan_INCLUDE_DIRS}    # Vulkan headers
    ${GLFW_INCLUDE_DIRS}      # GLFW headers (more robust way)
    ${glm_SOURCE_DIR}         # GLM headers
    ${CMAKE_SOURCE_DIR}/include  # If you have an include directory in your project
)

# Ensure the linker knows where to find GLFW
target_link_directories(GP2_VulkanTutorial PRIVATE 
    ${GLFW_SOURCE_DIR}/src
    _deps/glfw-build/src
)

# Ensure GLFW is copied to the output directory
add_custom_command(TARGET GP2_VulkanTutorial POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:glfw>
        $<TARGET_FILE_DIR:GP2_VulkanTutorial>
)

# Debugging messages
message(STATUS "Vulkan Include Directories: ${Vulkan_INCLUDE_DIRS}")
message(STATUS "GLFW Include Directories: ${GLFW_INCLUDE_DIRS}")
message(STATUS "GLM Include Directories: ${glm_SOURCE_DIR}")
