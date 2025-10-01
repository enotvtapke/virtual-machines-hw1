cmake_minimum_required(VERSION 4.0)
project(hw1)

set(CMAKE_CXX_STANDARD 20)

add_executable(cache_assoc cache_assoc.cpp)
add_executable(cache_size_vector cache_size_vector.cpp
        cache_assoc.cpp)
add_executable(main
        main.cpp
        cache_size.cpp
        cache_line_size.cpp
)

target_compile_options(main PRIVATE
        -Wall          # Enable all common warnings
        -Wextra        # Enable even more warnings
        -fmodules-ts
)