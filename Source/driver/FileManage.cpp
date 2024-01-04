#include "include/FileManager.hpp"
#include <filesystem>
#include <string>


void FileManager::FileManage_Print(void)
{
    struct directory_content_info dirContentInfo = GetDirectoryInfo();

    std::cout << "FileManage_Print" << std::endl;

    if(dirContentInfo.dirList.size() == 0 && dirContentInfo.fileList.size() == 0) return;

    if(dirContentInfo.dirList.size() > 0)
    {
        std::cout << "directory list" << std::endl;

        for(auto const& dir : dirContentInfo.dirList)
        {
            std::cout << "directory: " << dir.path << std::endl;
        }
    }

    if(dirContentInfo.fileList.size() > 0)
    {
        std::cout << "file list" << std::endl;

        for(auto const& file : dirContentInfo.fileList)
        {
            std::cout << "file: " << file.path << std::endl;
        }
    }
}

void FileManager::FileManage_Test(void)
{
    struct directory_content_info dirContentInfo = GetDirectoryInfo();

    std::filesystem::directory_entry dirEntry(std::filesystem::current_path());
    std::filesystem::directory_iterator dirIterator(dirEntry);

    for (auto const& dir_entry:  dirIterator)
    {
        if(dir_entry.is_directory())
        {
            std::cout << "directory: " << dir_entry.path() << std::endl;
            dirContentInfo.dirList.push_back({dir_entry.path().string(), dir_entry.path().filename().string(), "rwxrwxrwx"});

        }
        else
        {
            std::cout << "file: " << dir_entry.path() << std::endl;
            dirContentInfo.fileList.push_back({dir_entry.path().string(), dir_entry.path().filename().string(), dir_entry.path().extension().string(), std::to_string(dir_entry.file_size()), dir_entry.last_write_time(), "file", "root", "root", "rwxrwxrwx"});            
        }
    }

  SetDirectoryInfo(dirContentInfo);
  FileManage_Print();

  //m_DirectoryListing();

}


struct FileManager::directory_content_info FileManager::GetDirectoryInfo(void) const
{
    return m_dirContentInfo;
}

void FileManager::SetDirectoryInfo(struct FileManager::directory_content_info const &info)
{
    m_dirContentInfo = info;
}

/* 1. divide directory and files from input data.
   2. save each directory and files information. 
*/
void FileManager::m_DirectoryShowContent(void)
{
    std::cout << "m_DirectoryListing" << std::endl;
}


void FileManager::m_DirectoryRemove(struct FileManager::dirInfo const &info)
{
    std::cout << "m_DirectoryRemove" << std::endl;

    if(info.path.empty()) return;    

    try{
        std::filesystem::remove_all(info.path);
    }
    catch(const std::exception& e)
    {
         std::cerr << "Error removing directory: " << e.what() << std::endl;
    }
    
}
void FileManager::m_FileRemove(struct FileManager::fileInfo const &info)
{
    std::cout << "m_FileRemove" << std::endl;

    if(info.path.empty()) return;

    std::filesystem::remove(info.path);
}

void FileManager::m_DirectoryRename(struct FileManager::dirInfo const &info)
{
    std::cout << "m_DirectoryRename" << std::endl;

    if(info.path.empty()) return;

    std::string new_dir_name {""};
    //system(std::getline(std::cin, new_dir_name).c_str());
    
    try{
        std::filesystem::rename(info.name, new_dir_name);
    }
    catch(const std::exception& e)
    {
         std::cerr << "Error renaming directory: " << e.what() << std::endl;
    }
    
}

void FileManager::m_FileRename(struct FileManager::fileInfo const &info)
{
    std::cout << "m_FileRename" << std::endl;

    if(info.path.empty()) return;

    std::string new_file_name {""};
    //system(std::getline(std::cin, new_file_name).c_str());

    try{
        std::filesystem::rename(info.name, new_file_name);
    }
    catch(const std::exception& e)
    {
         std::cerr << "Error renaming file: " << e.what() << std::endl;
    }
}

void FileManager::m_DirectoryCreate(void)
{
    std::cout << "m_DirectoryCreate" << std::endl;
    //todo
}

void FileManager::m_FileCreate(void)
{
    std::cout << "m_FileCreate" << std::endl;
    //todo
}

#if 0 // TODO implement later
void FileManager::m_DirectoryCopy(void) {}
void FileManager::m_DirectoryMove(void) {}
void FileManager::m_DirectorySearch(void) {}
void FileManager::m_DirectoryPrint(void) {}
void FileManager::m_DirectoryHelp(void) {}
void FileManager::m_DirectoryExit(void) {}
#endif
