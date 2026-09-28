cmake_minimum_required(VERSION 3.20)

set(NAME Glint)


set(CMAKE_CXX_STANDARD 20)
project(${NAME} VERSION 0.0.1)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenGL REQUIRED)

################## Dependencies ##################
# GLFW
if (WIN32)
    # Prebuilt Windows binaries shipped in ext/
    set(GLFW_PATH ${PROJECT_SOURCE_DIR}/ext/glfw-3.4.bin.WIN64)
    set(GLFW_INCLUDE ${GLFW_PATH}/include)
    set(GLFW_LIB_DIR ${GLFW_PATH}/lib-vc2022)
    set(GLFW_LIBRARY glfw3)
else()
    # System package (e.g. libglfw3-dev on Debian/Ubuntu)
    find_package(glfw3 3.3 REQUIRED)
    set(GLFW_LIBRARY glfw)
endif()

# std::format needs GCC 13+ / MSVC 19.29+; fall back to {fmt} on older toolchains
include(CheckCXXSourceCompiles)
set(CMAKE_REQUIRED_FLAGS "-std=c++20")
if (MSVC)
    set(CMAKE_REQUIRED_FLAGS "/std:c++20")
endif()
check_cxx_source_compiles("
    #include <format>
    int main() { return std::format(\"{}\", 1).size() == 1 ? 0 : 1; }" GLINT_HAS_STD_FORMAT)
unset(CMAKE_REQUIRED_FLAGS)

if (NOT GLINT_HAS_STD_FORMAT)
    message(STATUS "std::format not available, using {fmt}")
    include(FetchContent)
    FetchContent_Declare(fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG 11.1.4
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(fmt)
endif()

# ImGui
set(IMGUI_PATH ${PROJECT_SOURCE_DIR}/ext/imgui)
file(GLOB_RECURSE IMGUI_SOURCES ${IMGUI_PATH}/*.cpp)


# GLAD
set(GLAD_PATH ${PROJECT_SOURCE_DIR}/ext/glad)
set(GLAD_INCLUDE ${GLAD_PATH}/include)
file(GLOB_RECURSE GLAD_SOURCES ${GLAD_PATH}/src/*.c)

# GLM
set(GLM_PATH ${PROJECT_SOURCE_DIR}/ext/glm)
set(GLM_INCLUDE ${GLM_PATH})
################## Dependencies ##################



################## Core ##################


# Core
# file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

# Source files
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE HEADERS ${PROJECT_SOURCE_DIR}/src/*.h)
file(GLOB_RECURSE SHADERS ${PROJECT_SOURCE_DIR}/shaders/*.vert ${PROJECT_SOURCE_DIR}/shaders/*.frag)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${SHADERS})
target_sources(${PROJECT_NAME} PRIVATE ${GLAD_SOURCES})
target_sources(${PROJECT_NAME} PRIVATE ${IMGUI_SOURCES})


target_compile_definitions(${PROJECT_NAME} PRIVATE ROOT="${PROJECT_SOURCE_DIR}")
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

# MSVC defines _DEBUG for debug builds, other compilers don't
if (NOT MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:_DEBUG>)
endif()

if (WIN32)
    target_link_directories(${PROJECT_NAME} PUBLIC ${GLFW_LIB_DIR})
endif()
target_link_libraries(${PROJECT_NAME}
    ${GLFW_LIBRARY}
    OpenGL::GL
    ${CMAKE_DL_LIBS}
)

if (NOT GLINT_HAS_STD_FORMAT)
    target_link_libraries(${PROJECT_NAME} fmt::fmt)
endif()


SOURCE_GROUP(0_External REGULAR_EXPRESSION "^${PROJECT_SOURCE_DIR}/ext/.*" )
SOURCE_GROUP(Shaders REGULAR_EXPRESSION ".*[.](vert|frag)$" )
SOURCE_GROUP(Core REGULAR_EXPRESSION "^${PROJECT_SOURCE_DIR}/src/Core/.*$" )
SOURCE_GROUP(Graphics REGULAR_EXPRESSION "^${PROJECT_SOURCE_DIR}/src/Core/.*$" )

target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/ext
    ${GLAD_INCLUDE}
    ${GLM_INCLUDE}
)
if (WIN32)
    target_include_directories(${PROJECT_NAME} PUBLIC ${GLFW_INCLUDE})
endif()

# Enable warnings
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /permissive-)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()

################## Core ##################
