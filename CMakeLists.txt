cmake_minimum_required(VERSION 4.0)
project(hw1)

set(CMAKE_CXX_STANDARD 20)

add_executable(main
        main.cpp
        jump_analisis.cpp
        header.h
)

add_executable(anal
        jump_analisis.cpp
        header.h
)

target_compile_options(main PRIVATE
        -O2
)
