cmake_minimum_required(VERSION 3.27)
project(Archivator)

set(CMAKE_CXX_STANDARD 23)


add_subdirectory(src)
#add_subdirectory(tst)
add_subdirectory(lib/googletest)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=leak -fsanitize=undefined -g3")