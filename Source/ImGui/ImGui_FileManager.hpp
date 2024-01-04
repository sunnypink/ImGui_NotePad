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
//#include "../Headers/FileManager.hpp"
#include "../Headers/imgui.h"
#include <unistd.h>
#include <iostream>
#include <filesystem>
#include <string>

#ifndef IMGUI_DISABLE



class ImGui_FileExplore{
private:
#if 1
	struct directory_Info{
		std::string current_directory_path; // since c++17
		std::string current_folder_name;
		std::string current_file_type;
		std::string modified_file_time; // need to check. time variable is possible
	};
#endif
	directory_Info m_dir_info;
	void m_ImGui_ReadDirectoryList(directory_Info & dir_info);
	void m_ImGui_SelectDirectoryPath(char *selDir);
	void m_ImGui_PrintLog(char *log);

public:

	//directory_Info m_dir_info;
	//FileManager::directory_content_info m_imgui_dirContentInfo;
	//ImGui_FileExplore(){	}; // Constructor
	//~ImGui_FileExplore(); //Destructor

    void ImGui_FileWindowDraw(const char* title, bool* p_open); // Draw a Window

};


#endif
#endif /* SOURCE_IMGUI_IMGUI_FILEMANAGER_HPP_ */
