cmake_minimum_required(VERSION 3.15)

project(ToricCode LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Automatically collect all .cpp files inside src/
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

add_executable(ToricCode
    ${SRC_FILES}
)

target_compile_definitions(ToricCode PRIVATE GL_SILENCE_DEPRECATION)
target_compile_definitions(ToricCode PRIVATE SHADER_DIR="${CMAKE_BINARY_DIR}/Shaders/")

target_include_directories(ToricCode PRIVATE "${CMAKE_SOURCE_DIR}")
target_include_directories(ToricCode PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

# -------------------------------------------------------------------
# HYBRID BUILD: Detects if compiling with Emscripten (Web) vs Desktop
# -------------------------------------------------------------------
if(EMSCRIPTEN)
    # FORCES CMake to generate an HTML bundle (fixes the missing *.html error)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")

    target_compile_options(ToricCode PRIVATE "-sUSE_GLFW=3")
    target_link_libraries(ToricCode PRIVATE "-sUSE_GLFW=3")
else()
    # DESKTOP BUILD (Your original local Mac configuration)
    find_package(OpenGL REQUIRED)
    find_package(GLEW REQUIRED)
    find_package(glfw3 REQUIRED)

    target_link_libraries(ToricCode PRIVATE
        OpenGL::GL
        GLEW::GLEW
        glfw
    )
endif()

# Copy shader folder into build folder
file(COPY ${CMAKE_SOURCE_DIR}/Shaders
     DESTINATION ${CMAKE_BINARY_DIR})
