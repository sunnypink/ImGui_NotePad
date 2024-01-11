#ifndef FILEMANAGER_HPP
#define FILEMANAGER_HPP

#include <string>
#include <filesystem>
#include <iostream>
#include <chrono>
#include <form.h>
#include <vector>

class FileManager
{
public:
    struct fileInfo
    {
        std::string path;
        std::string name;
        std::string extension;
        std::string size;
        std::filesystem::file_time_type last_modified_time;
        std::string type;
        std::string owner;
        std::string ownerGroup;
        std::string permission;
    };
   struct dirInfo
    {
    	std::filesystem::path currentPath;
    	std::filesystem::path subFolderPath;
        std::string subPath;
        std::string name;
		std::string extension;
		int number_subitem;
        std::filesystem::file_time_type last_modified_time;
        std::string permission;
    };

    struct directory_content_info
    {
	    std::vector<std::filesystem::path> rootPathList;
        std::vector<struct dirInfo> dirList;
        std::vector<struct fileInfo> fileList;
    };

private:

    void ReadDirectoryList(void);
    void DirectoryShowContent(void);
    void DirectoryRemove(struct dirInfo const &info);
    void FileRemove(struct fileInfo const &info);
    void DirectoryCreate(void);
    void FileCreate(void);
    void DirectoryRename(struct dirInfo const &info);
    void FileRename(struct fileInfo const &info);
   // const int CheckSubItemCount(struct dirInfo const &info);
    //void iterateDirectory(const std::filesystem::path& dirPath,int* itemCount);
    void PreviewSubItem(std::filesystem::path const &path,int* itemCount);
    void RootPathList(std::filesystem::path const &currentpath,std::vector<std::filesystem::path>& rootPathList);

    
#if 0 // TODO implement later    
    void m_DirectoryCopy(void);
    void m_DirectoryMove(void);        
    void m_DirectorySearch(void);
    void m_DirectoryPrint(void);
    void m_DirectoryHelp(void);
    void m_DirectoryExit(void);
#endif

    struct directory_content_info m_dirContentInfo;

public:

    

    const struct directory_content_info& GetDirectoryInfo(void);
    void SetDirectoryInfo(struct directory_content_info const &info);

        FileManager()
        {
        	ReadDirectoryList();
        };

    //void FileManage_Test(void);
    //void FileManage_Print(void);

};

#endif // FILEMANAGER_HPP
