cmake_minimum_required(VERSION 3.27)
project(Archivator)
set(CMAKE_CXX_COMPILER "g++-11")

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

find_package(OpenCV REQUIRED)

include_directories(../src/controller)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(MyExec
        ../src/main.cpp
        ../src/controller/Controller.cpp
        ../src/controller/Controller.h
)

target_link_libraries(MyExec PRIVATE ${OpenCV_LIBS})
#add_subdirectory(src)

#add_subdirectory(tst)
#add_subdirectory(lib/googletest)
#add_subdirectory(lib/AudioPart)
#add_subdirectory(lib/ImagePart)
#add_subdirectory(lib/VideoPart)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=leak -fsanitize=undefined -g3")
