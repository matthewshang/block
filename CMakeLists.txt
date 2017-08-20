cmake_minimum_required (VERSION 2.6)
project (block)

include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${CMAKE_SOURCE_DIR}/libs)

file (GLOB block_SRCS
	  "src/*.h"
	  "src/*.cpp"
	  "src/*.c"
)

set (CMAKE_CXX_STANDARD 17)

add_executable (block ${block_SRCS})

if (APPLE)
	include_directories (/System/Library/Frameworks)
	find_library (COCOA_LIBRARY Cocoa)
	find_library (OpenGL_LIBRARY OpenGL)
	find_library (IOKit_LIBRARY IOKit)
	find_library (CoreVideo_LIBRARY CoreVideo)
	mark_as_advanced (COCOA_LIBRARY OpenGL_LIBRARY 
					  IOKit_LIBRARY CoreVideo_LIBRARY)
	set (APPLE_LIBS ${COCOA_LIBRARY} ${OpenGL_LIBRARY} 
		 ${IOKit_LIBRARY} ${CoreVideo_LIBRARY})
endif (APPLE)

target_link_libraries (block glfw3 ${APPLE_LIBS})