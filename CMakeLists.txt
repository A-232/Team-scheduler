# Формальность, указывается минимальная версия CMake.
cmake_minimum_required(VERSION 3.15)

# Задаётся имя проекта и что он использует язык С++.
project(Team_Scheduler LANGUAGES CXX)

# Определяю что буду использовать стандарт C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#Тут указывается какие файлы будут компилироваться.И как будет назван итоговый ехе файл.
add_executable(Team_Scheduler main.cpp)

# === ВАЖНО: Статическая линковка ===
# Эта строчка заставит компилятор "вшить" все библиотеки внутрь exe файла,
# чтобы на другом компьютере не вылезло "не хватает msvcp140.dll"
set_property(TARGET Team_Scheduler PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

