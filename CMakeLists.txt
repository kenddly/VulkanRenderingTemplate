cmake_minimum_required(VERSION 3.20)

# write the cmakebinarydir to std
message(STATUS "CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")

if(EXISTS "${CMAKE_BINARY_DIR}/generators/conan_toolchain.cmake")
    include("${CMAKE_BINARY_DIR}/generators/conan_toolchain.cmake")
endif()

project(
    VulkanStarter
    DESCRIPTION "A template for Vulkan C++ projects with GLFW, GLM, ImGUI and Vulkan using Conan 2 and CMake"
    LANGUAGES CXX
)

# ---------------------------
# C++ Standard
# ---------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------
# Packages provided by Conan
# ---------------------------
# These names come from CMakeDeps generated .cmake files
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(imgui REQUIRED)
find_package(VulkanMemoryAllocator REQUIRED)
find_package(glslang REQUIRED)

# Vulkan (native system SDK)
find_package(Vulkan REQUIRED)

# ---------------------------
# Source files
# ---------------------------
file(GLOB_RECURSE PROJECT_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp" "${CMAKE_SOURCE_DIR}/bindings/*.cpp")
file(GLOB_RECURSE PROJECT_HEADERS "${CMAKE_SOURCE_DIR}/include/*.hpp" "${CMAKE_SOURCE_DIR}/bindings/*.h")

# ---------------------------
# Create main library
# ---------------------------
add_library(${PROJECT_NAME} OBJECT
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/bindings
)

# Needed for ImGui Vulkan backend
target_compile_definitions(${PROJECT_NAME} PUBLIC IMGUI_IMPL_VULKAN)

# Link Conan libraries
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Vulkan::Vulkan
        glfw
        glm::glm
        imgui::imgui
        GPUOpen::VulkanMemoryAllocator
        glslang::glslang
)

# ---------------------------
# Shader compilation
# ---------------------------
file(GLOB_RECURSE SHADERS
    "${CMAKE_SOURCE_DIR}/assets/shaders/*.frag"
    "${CMAKE_SOURCE_DIR}/assets/shaders/*.vert"
)

include(cmake/tools/compile-shader.cmake)
compile_shaders(TARGETS ${SHADERS})

# ---------------------------
# Optional Executable
# ---------------------------
option(BUILD_APPS "Build example application" ON)

if(BUILD_APPS)
    add_executable(${PROJECT_NAME}Standalone app/main.cpp)
    target_link_libraries(${PROJECT_NAME}Standalone PRIVATE ${PROJECT_NAME})
endif()

# ---------------------------
# Tests
# ---------------------------
option(BUILD_TESTING "Enable tests with doctest" OFF)

if(BUILD_TESTING)
    include(CTest)
    enable_testing()
    add_subdirectory(test)
endif()

# ---------------------------
# Documentation (Doxygen)
# ---------------------------
option(BUILD_DOCS "Enable Doxygen docs" OFF)

if(BUILD_DOCS)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        add_subdirectory(docs)
    endif()
endif()

