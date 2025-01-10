#imgui

cmake_minimum_required(VERSION 3.20)
set(PROJECTNAME imgui)

add_library(${PROJECTNAME}
	imconfig.h
	imgui.cpp
	imgui.h
	imgui_draw.cpp
	imgui_impl_dx11.cpp
	imgui_impl_dx11.h
	imgui_internal.h
	imgui_tables.cpp
	imgui_widgets.cpp
	imstb_rectpack.h
	imstb_textedit.h
	imstb_truetype.h
	imgui_impl_win32.cpp
	imgui_impl_win32.h
	imspinner.h
	)
