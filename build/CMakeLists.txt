cmake_minimum_required(VERSION 2.8.6)
if(WIN32)
	add_definitions(-DWINDOWS -DMULTIPLE_SCORE_ENTRY -DTERM_DARK_BACKGROUND)
	else()
	add_definitions(-DNIX -DMULTIPLE_SCORE_ENTRY -DTERM_DARK_BACKGROUND)
endif()
project(Larn)

find_package(Curses REQUIRED)

add_definitions(-DEXTRA)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -pipe -Wall -Wextra -pedantic -Wpointer-arith -Wstrict-prototypes -Wmissing-prototypes -Wdeclaration-after-statement -Wshadow -Wmissing-declarations -Wold-style-definition -Wredundant-decls -g -std=c18")

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -pipe -Wall -fomit-frame-pointer -std=c18")

include_directories(../src/includes)
include_directories( ${CURSES_INCLUDE_DIRS})
file(GLOB SOURCES "../src/*.c")
add_executable(larn ${SOURCES})
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lm")
target_link_libraries(larn ${CURSES_LIBRARIES} ${CMAKE_DL_LIBS})
