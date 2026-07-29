cmake_minimum_required(VERSION 3.15)
project(ToricCode LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

add_executable(ToricCode ${SRC_FILES})

target_compile_definitions(ToricCode PRIVATE GL_SILENCE_DEPRECATION)
target_include_directories(ToricCode PRIVATE "${CMAKE_SOURCE_DIR}")
target_include_directories(ToricCode PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")

    # Shaders now live INSIDE the wasm virtual filesystem, at /Shaders/
    target_compile_definitions(ToricCode PRIVATE SHADER_DIR="/Shaders/")

    target_link_options(ToricCode PRIVATE
        "-sUSE_GLFW=3"
        "-sUSE_WEBGL2=1"
        "-sFULL_ES3=1"
        "-sALLOW_MEMORY_GROWTH=1"
        "SHELL:--embed-file ${CMAKE_SOURCE_DIR}/Shaders@/Shaders"
    )
else()
    # Desktop reads shaders from the build folder on disk
    target_compile_definitions(ToricCode PRIVATE SHADER_DIR="${CMAKE_BINARY_DIR}/Shaders/")

    find_package(OpenGL REQUIRED)
    find_package(GLEW REQUIRED)
    find_package(glfw3 REQUIRED)
    target_link_libraries(ToricCode PRIVATE
        OpenGL::GL
        GLEW::GLEW
        glfw
    )

    file(COPY ${CMAKE_SOURCE_DIR}/Shaders DESTINATION ${CMAKE_BINARY_DIR})
endif()