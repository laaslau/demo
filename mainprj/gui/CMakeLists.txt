#Gui
cmake_minimum_required(VERSION 3.20)
set(PROJECTNAME gui)

add_library(${PROJECTNAME}
	GuiContainer.cpp
	GuiContainer.h
	IconsFontAwesome5.h
	d3d11helper.cpp
	d3d11helper.h
	d3d11imageRender.cpp
	d3d11imageRender.h
	GuiHelper.cpp
	GuiHelper.h
	fontdata.cpp
	fontdata.h
	GuiVideoWindow.cpp
	GuiVideoWindow.h
	)

target_include_directories(${PROJECTNAME} PRIVATE
	"${PROJECT_SOURCE_DIR}/ImGui"
	"${PROJECT_SOURCE_DIR}/Toolbox"
	"${PROJECT_SOURCE_DIR}/common"
	)
