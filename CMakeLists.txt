cmake_minimum_required(VERSION 4.0)
project(hw1)

set(CMAKE_CXX_STANDARD 20)

add_executable(main
        main.cpp
        cache_size.cpp
        cache_line_size.cpp
        cache_assoc.cpp
)

target_compile_options(main PRIVATE
        -O2
)