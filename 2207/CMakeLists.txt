cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)

project(lingshen_2207)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls")

find_package(Threads REQUIRED)

add_subdirectory(2)
add_subdirectory(3)