cmake_minimum_required(VERSION 3.6)

project(MyArch)

set(CMAKE_CXX_COMPILER "g++-11")

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
add_executable(MyExec main.cpp
)
target_link_libraries(MyExec PRIVATE ${OpenCV_LIBS})
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=leak -fsanitize=undefined -fsanitize=address -pedantic -g3")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast"