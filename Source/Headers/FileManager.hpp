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
        std::filesystem::file_time_type::clock::time_point time;
        std::string type;
        std::string owner;
        std::string ownerGroup;
        std::string permission;
    };
   struct dirInfo
    {
        std::string path;
        std::string name;
        std::string permission;
    };

    struct directory_content_info
    {
        std::vector<struct dirInfo> dirList;
        std::vector<struct fileInfo> fileList;
    };

private:

    void m_DirectoryShowContent(void);
    void m_DirectoryRemove(struct dirInfo const &info);
    void m_FileRemove(struct fileInfo const &info);
    void m_DirectoryCreate(void);
    void m_FileCreate(void);
    void m_DirectoryRename(struct dirInfo const &info);
    void m_FileRename(struct fileInfo const &info);

    
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

    

    struct directory_content_info GetDirectoryInfo(void) const;
    void SetDirectoryInfo(struct directory_content_info const &info);

        FileManager() : m_dirContentInfo(directory_content_info{})
        {};

    void FileManage_Test(void);
    void FileManage_Print(void);

};

#endif // FILEMANAGER_HPP
