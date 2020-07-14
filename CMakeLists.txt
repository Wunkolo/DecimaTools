cmake_minimum_required( VERSION 3.8.0 )
project( DecimaTools )

set( CMAKE_CXX_STANDARD 11 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

set( CMAKE_COLOR_MAKEFILE ON )
set( CMAKE_VERBOSE_MAKEFILE ON )
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

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


if( MSVC )
	target_compile_options( Decima PUBLIC /W3 )
elseif( CMAKE_COMPILER_IS_GNUCXX )
	target_compile_options( Decima PUBLIC -Wall )
	target_compile_options( Decima PUBLIC -Wextra )
endif()


add_subdirectory(extern/mio)

add_executable(
	Archive
	tools/Archive.cpp
)

target_link_libraries(
	Archive
	PRIVATE
	Decima
	mio
)