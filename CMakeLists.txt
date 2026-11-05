cmake_minimum_required(VERSION 3.28.3)
project(OS_sem3 C)

add_compile_options(-fsanitize=address -static-libasan)
add_link_options(-fsanitize=address -static-libasan)

set(CMAKE_C_STANDARD 11)

add_subdirectory(lab2/src)
add_subdirectory(lab3/src)
add_subdirectory(lab4/src)