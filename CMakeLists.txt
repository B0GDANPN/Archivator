cmake_minimum_required(VERSION 3.31)

project(Archivator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpedantic")
find_package(OpenCV REQUIRED)
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
file(GLOB_RECURSE SOURCE_FILES
        "src/*.cpp"
)
add_executable(MyExec ${SOURCE_FILES})
target_include_directories(MyExec PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
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
