cmake_minimum_required(VERSION 3.6)

project(Archivator)

set(CMAKE_CXX_COMPILER "g++-11")

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
find_package(OpenCV REQUIRED)
include_directories(../src/controller)
include_directories(${OpenCV_INCLUDE_DIRS})

find_package(SFML COMPONENTS audio graphics window system REQUIRED)
include_directories(${SFML_INCLUDE_DIR})


add_executable(MyExec
  ../src/main.cpp
)

target_link_libraries(MyExec PRIVATE ${OpenCV_LIBS} sfml-graphics sfml-window sfml-system)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=leak -fsanitize=undefined -fsanitize=address -pedantic -g3")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")
