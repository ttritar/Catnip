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



# COMPILING SHADERS
#-------------------

# Define directories
set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Shaders")
set(SHADER_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/Shaders")

# Get GLSL shader files
file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${SHADER_SOURCE_DIR}/*.frag"
    "${SHADER_SOURCE_DIR}/*.vert")

# Find the glslc compiler
find_program(GLSLC_EXECUTABLE NAMES glslc HINTS ${Vulkan_GLSLC_EXECUTABLE})

# Compile GLSL to SPIR-V
foreach(GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${SHADER_BINARY_DIR}/${FILE_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADER_BINARY_DIR}"
        COMMAND ${GLSLC_EXECUTABLE} ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL}
    )
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

# Create a custom target for shaders
add_custom_target(
    Shaders
    DEPENDS ${SPIRV_BINARY_FILES}
)

# Add Shader executable dependency
add_dependencies(GP2_VulkanTutorial Shaders)

# Add shader files to your target; they will not be compiled
target_sources(GP2_VulkanTutorial PRIVATE ${GLSL_SOURCE_FILES})

# For each shader file, mark it as HEADER_FILE_ONLY
set_source_files_properties(${GLSL_SOURCE_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)

# Organize shader files in the IDE
source_group("Shaders" FILES ${GLSL_SOURCE_FILES})

file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})

