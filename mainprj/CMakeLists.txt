cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

set(PROJECTNAME cvbench)

project(${PROJECTNAME} LANGUAGES C CXX)

# solution settings

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
set(CMAKE_MFC_FLAG 1)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_compile_definitions(${PROJECTNAME} PUBLIC
	UNICODE 
	_UNICODE
	)

# solution, executable + lib

add_executable(${PROJECTNAME} 
	main.cpp
	)

add_subdirectory(common)
add_subdirectory(toolbox)
add_subdirectory(imgui)
add_subdirectory(gui)
	
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${PROJECTNAME})	


target_include_directories(${PROJECTNAME} PRIVATE
	common
	toolbox
	imgui
	gui
	)
	
target_link_libraries(${PROJECTNAME} 
	common
	toolbox
	imgui
	gui
	)


