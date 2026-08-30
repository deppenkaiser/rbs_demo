cmake_minimum_required(VERSION 3.10)
project(rbs_demo LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)

add_compile_definitions(_POSIX_C_SOURCE=200809L)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(rbs_demo)
add_subdirectory(../../libraries/rbs ${CMAKE_BINARY_DIR}/rbs)

target_sources(rbs_demo PRIVATE
	src/main.c
	src/app.c
)

# src/ deckt das lokale "app.h"; Alle Bibliotheks-Includes (<rbs/rbs.h>,
# <sm/sm.h>, <logging/logging.h>) liefert die rbs-Bibliothek via PUBLIC-Links.
target_include_directories(rbs_demo PRIVATE
	src
)

target_link_libraries(rbs_demo
	PUBLIC
		m
		rbs
)
