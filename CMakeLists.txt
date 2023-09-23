cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

project(tetris_game LANGUAGES C)

add_executable(tetris tetris.c)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})
target_link_libraries(tetris ${CURSES_LIBRARIES})
