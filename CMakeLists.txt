cmake_minimum_required(VERSION 3.10)
project(FakeVim)

set(CMAKE_CXX_STANDARD 17)


set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)
if(!CURSES_FOUND)
    message("Curses Not found")
else()
    message("Curses: ${CURSES_INCLUDE_DIR}:${CURSES_LIBRARIES}")
endif()

link_libraries(${CURSES_LIBRARIES})
include_directories(${CURSES_INCLUDE_DIR})
add_executable(FakeVim main.cpp)