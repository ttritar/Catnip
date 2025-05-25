cmake_minimum_required(VERSION 3.3)

project(GP2_VulkanTutorial)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)


# Define the executable
add_executable(GP2_VulkanTutorial 
    src/main.cpp 
    src/core/Renderer.cpp
    src/core/Window.cpp
    src/vulkan/Device.cpp src/vulkan/SwapChain.cpp src/vulkan/Descriptors.cpp
    src/vulkan/Buffer.cpp src/vulkan/UniformBuffer.cpp  src/vulkan/CommandBuffer.cpp  
    src/vulkan/Pipeline.cpp
    src/vulkan/GeometryPass.cpp 
    src/vulkan/Scene.cpp src/vulkan/Model.cpp src/vulkan/Mesh.cpp src/vulkan/Image.cpp src/core/Camera.cpp 
  )



#===================
# FETCH CONTENT
#===================

include(FetchContent)

# GLFW
#-----------
FetchContent_Declare(
    GLFW
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)
FetchContent_MakeAvailable(GLFW) 


# glm
#-----------
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0.9.9.8
)
FetchContent_MakeAvailable(glm)


# stb
#-----------
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
)
FetchContent_MakeAvailable(stb)


# tiny obj
#-----------
FetchContent_Declare(
    tiny
    GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
    GIT_TAG v2.0.0rc13
)
FetchContent_MakeAvailable(tiny)


# assimp
#-----------
FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG v5.4.3 
)
FetchContent_MakeAvailable(assimp)


# models
#-----------
FetchContent_Declare(
    model
    GIT_REPOSITORY https://github.com/ttritar/GP2_Models.git
    GIT_TAG main 
)
FetchContent_MakeAvailable(model)


# vma
#-----------
FetchContent_Declare(
        vma
        GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
        GIT_TAG v3.2.1
)

FetchContent_GetProperties(vma)

if (NOT vma_POPULATED)
    FetchContent_MakeAvailable(vma)
endif()



# Link necessary libraries
target_link_libraries(GP2_VulkanTutorial PRIVATE glfw glm Vulkan::Vulkan assimp)

# Include directories
target_include_directories(GP2_VulkanTutorial PRIVATE 
    ${Vulkan_INCLUDE_DIRS}  
    ${GLFW_INCLUDE_DIRS}     
    ${glm_SOURCE_DIR}     
    ${stb_SOURCE_DIR}   
    ${CMAKE_SOURCE_DIR}/include  
    ${tiny_SOURCE_DIR}   
    ${assimp_SOURCE_DIR} 
    ${vma_SOURCE_DIR}
)

target_link_directories(GP2_VulkanTutorial PRIVATE 
    ${GLFW_SOURCE_DIR}/src
    _deps/glfw-build/src
)

add_custom_command(TARGET GP2_VulkanTutorial POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:glfw>
        $<TARGET_FILE_DIR:GP2_VulkanTutorial>
)

set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT GP2_VulkanTutorial)



#===================
# COMPILATION 
#===================


# SHADERS
#---------

find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)

# Define shader source and destination directories
set(SHADER_SRC_DIR "${PROJECT_SOURCE_DIR}/shaders")
set(COMPILED_SHADER_DIR "${PROJECT_SOURCE_DIR}/shaders/compiled_shaders")
set(OUTPUT_SHADER_DIR "${CMAKE_BINARY_DIR}/shaders")

# Ensure the compiled shader directory exists
file(MAKE_DIRECTORY ${COMPILED_SHADER_DIR})

file(GLOB_RECURSE GLSL_SOURCE_FILES
        "${SHADER_SRC_DIR}/*.frag"
        "${SHADER_SRC_DIR}/*.vert"
        "${SHADER_SRC_DIR}/*.comp"
)

set(SPIRV_BINARY_FILES "")

foreach(GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${COMPILED_SHADER_DIR}/${FILE_NAME}.spv")

    message(STATUS "Compiling shader: ${GLSL} -> ${SPIRV}")

    add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
            DEPENDS ${GLSL}
    )

    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach()

add_custom_target(CompileShaders DEPENDS ${SPIRV_BINARY_FILES})

file(MAKE_DIRECTORY ${OUTPUT_SHADER_DIR})

foreach(SPV ${SPIRV_BINARY_FILES})
    get_filename_component(FILE_NAME ${SPV} NAME)
    add_custom_command(
            TARGET CompileShaders POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SPV} ${OUTPUT_SHADER_DIR}/${FILE_NAME}
            COMMENT "Copying ${FILE_NAME} to output shaders directory"
    )
endforeach()

source_group("shaders" FILES ${GLSL_SOURCE_FILES})
add_dependencies(${PROJECT_NAME} CompileShaders)



# RESOURCES
#---------

# Copy resources into build directory
set(RESOURCE_SRC_DIR "${PROJECT_SOURCE_DIR}/resources")
set(RESOURCE_DST_DIR "${CMAKE_BINARY_DIR}/resources")

add_custom_target(CopyResources ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${RESOURCE_SRC_DIR} ${RESOURCE_DST_DIR}
        COMMENT "Copying resources folder to build directory"
)

# Copy downloaded models to resources/models
add_custom_target(CopyModels ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${model_SOURCE_DIR} ${RESOURCE_DST_DIR}
    COMMENT "Copying models from GP2_Models repo to resources/models"
)

add_dependencies(${PROJECT_NAME} CopyModels)
add_dependencies(${PROJECT_NAME} CopyResources)