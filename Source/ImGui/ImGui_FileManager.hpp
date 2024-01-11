/*
 * ImGui_FileManager.hpp
 *
 *  Created on: Dec 28, 2023
 *      Author: sunny
 */

#ifndef SOURCE_IMGUI_IMGUI_FILEMANAGER_HPP_
#define SOURCE_IMGUI_IMGUI_FILEMANAGER_HPP_

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif
#include "../Headers/FileManager.hpp"
#include "../Headers/imgui.h"

#include <unistd.h>
#include <iostream>
#include <filesystem>
#include <string>

#ifndef IMGUI_DISABLE

class ImGui_FileExplore{

private:

	FileManager::directory_content_info m_gui_dirContentInfo;

    // Using those as a base value to create width/height that are factor of the size of our font
    const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
    const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

    void ImGui_ReadDirectoryList(FileManager::directory_content_info &dirContentInfoc);
	void ImGui_SelectDirectoryPath(FileManager::directory_content_info &dirContentInfo);
	void ImGui_PrintLog(char *log);
	const char* to_string(const std::filesystem::file_time_type& ftime);
	char time_buffer[80];

#if 0
	char *m_gui_extension;
	char *m_gui_size;
	char *m_gui_time;
	char *m_gui_type;
	char *m_gui_owner;
	char *m_gui_ownerGroup;
	char *m_gui_permission;
#endif

public:
	FileManager fileManager; //FileeManager Class Object

	//FileManager::directory_content_info m_imgui_dirContentInfo;
	ImGui_FileExplore();	 // Constructor

	//~ImGui_FileExplore(); //Destructor

    void ImGui_FileWindowDraw(const char* title, bool* p_open); // Draw a Window
	void ImGui_FileManageBuildList(void);


};


#endif
#endif /* SOURCE_IMGUI_IMGUI_FILEMANAGER_HPP_ */
