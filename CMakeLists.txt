cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(bst LANGUAGES C CXX VERSION 0.1)

set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES})
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})

# Compile Options
add_compile_options(-O2 -Wall -Wextra -std=c++14 -Wold-style-cast -Woverloaded-virtual -Wsign-promo  -Wctor-dtor-privacy -Wnon-virtual-dtor  -Weffc++ -pedantic)
add_compile_options(-fdiagnostics-color=always)

# files to compile
add_executable(driver_c ./src/driver.cpp ./src/Support.cpp)
add_executable(driver_c_2 ./src/driver2.cpp ./src/Support.cpp)
add_executable(custom ./src/custom.cpp ./src/Support.cpp)
