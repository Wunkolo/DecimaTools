cmake_minimum_required( VERSION 3.8.0 )
project( DecimaTools )

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

set( CMAKE_COLOR_MAKEFILE ON )
set( CMAKE_VERBOSE_MAKEFILE ON )
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

### External dependencies
add_subdirectory(extern)

### libDecima
add_library(
	Decima
	source/Decima/Decima.cpp
)
target_include_directories(
	Decima
	PUBLIC
	include
)
target_link_libraries(
	Decima
	PUBLIC
	mio
)


if( MSVC )
	target_compile_options( Decima PUBLIC /W3 )
elseif( CMAKE_COMPILER_IS_GNUCXX )
	target_compile_options( Decima PUBLIC -Wall )
	target_compile_options( Decima PUBLIC -Wextra )
	target_compile_options( Decima PUBLIC -march=skylake )
endif()


## Archive
add_executable(
	Archive
	tools/Archive.cpp
)
target_link_libraries(
	Archive
	PRIVATE
	Decima
)

## Dump
add_executable(
	Dump
	tools/Dump.cpp
)
target_link_libraries(
	Dump
	PRIVATE
	Decima
)