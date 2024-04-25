cmake_minimum_required(VERSION 3.6)

project(MyArch)

set(CMAKE_CXX_COMPILER "g++-11")

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")

find_package(OpenCV REQUIRED)

include_directories(../src/controller)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(MyExec
  ../src/main.cpp
  ../src/controller/Controller.h
)

target_link_libraries(MyExec PRIVATE ${OpenCV_LIBS})