cmake_minimum_required(VERSION 3.10)
project(MultiLanguageCompiler)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB_RECURSE SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

add_executable(compiler ${SOURCE_FILES})