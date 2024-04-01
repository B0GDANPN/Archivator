cmake_minimum_required(VERSION 3.27)
project(TestFractal)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# Create a sources variable with a link to all cpp files to compile
set(SOURCES
        main.cpp
        Decoder.cpp
        Encoder.cpp
        IFSTransform.cpp
        QuadTreeEncoder.cpp
        Image.cpp
)
add_executable(TestFractal ${SOURCES})

target_include_directories(TestFractal
        PRIVATE
        ${PROJECT_SOURCE_DIR}
)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=leak -fsanitize=undefined -fsanitize=address -pedantic -g3")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")
