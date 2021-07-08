cmake_minimum_required(VERSION 3.16)
project(Algorithm)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined -Wall")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined -Wall -Werror -Wextra")

add_executable(main main.cpp)
add_executable(graph_generator graph_generator.cpp)