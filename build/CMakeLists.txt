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
if (UNIX)
    # Find the package for GTK
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK REQUIRED gtk+-3.0)

    # Include the GTK headers
    include_directories(${GTK_INCLUDE_DIRS})
endif (UNIX)

add_executable(MyExec
  ../src/main.cpp
)
if (UNIX)
    # Specify the compile flags
    target_compile_options(MyExec PUBLIC ${GTK_CFLAGS_OTHER})
    target_link_libraries(MyExec ${OpenCV_LIBS} sfml-graphics sfml-window sfml-system ${GTK_LIBRARIES})
endif (UNIX)
If(WIN32)
    target_link_libraries(MyExec ${OpenCV_LIBS} sfml-graphics sfml-window sfml-system)
endif (WIN32)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=leak -fsanitize=undefined -fsanitize=address -pedantic -g3")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")
