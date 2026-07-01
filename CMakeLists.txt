cmake_minimum_required(VERSION 3.20)
project(EnGl LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY         ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/Release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_BINARY_DIR}/bin/Debug)

file(GLOB_RECURSE ALL_SOURCES CONFIGURE_DEPENDS
    "src/*.cpp"
    "src/*.c"
    "src/*.h"
)

list(FILTER ALL_SOURCES EXCLUDE REGEX ".*/src/resources/assets/.*")

add_executable(${PROJECT_NAME} ${ALL_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/imgui
    ${CMAKE_SOURCE_DIR}/src/imgui/backends
    ${CMAKE_SOURCE_DIR}/include
)

target_link_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/lib
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw3
    opengl32
    assimp-vc143-mt
)

target_compile_options(${PROJECT_NAME} PRIVATE
    /utf-8
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/lib/assimp-vc143-mt.dll
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/resources/assets
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/resources/shaders
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources/shaders
)