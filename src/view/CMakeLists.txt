cmake_minimum_required(VERSION 3.5)

project(SFMLProject)

# Установка пути к SFML
set(SFML_DIR "SFML-2.6.1")

# Поиск пакета SFML
find_package(SFML 2.6 COMPONENTS graphics audio REQUIRED PATHS ${CMAKE_SOURCE_DIR})

# Указываем путь к заголовочным файлам SFML
include_directories("D:/Projects/Cpp/view/SFML-2.6.1/include")

# Создаем исполняемый файл
add_executable(${PROJECT_NAME} main.cpp)

# Линковка с библиотеками SFML
target_link_libraries(${PROJECT_NAME} sfml-graphics sfml-window sfml-system ole32)

