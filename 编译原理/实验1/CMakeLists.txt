cmake_minimum_required(VERSION 3.16)
project(lab01)
set(CMAKE_CXX_STANDARD 14)

include(FindFLEX)
if(FLEX_FOUND)
    message("Info: flex found!")
else()
    message(FATAL_ERROR "Error: flex not found!")
endif()

include_directories(${CMAKE_SOURCE_DIR})
FLEX_TARGET(MyScanner lexer.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)

add_compile_options(-pedantic)
add_executable(Compilerlab1
        main.cpp
        parser.cpp
        codegen.cpp
        ${FLEX_MyScanner_OUTPUTS}
)
target_compile_features(Compilerlab1 PRIVATE cxx_std_14)