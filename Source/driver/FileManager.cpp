
#include "../Headers/FileManager.hpp"
#include <filesystem>
#include <string>

#if 0
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
/*Just for Testing*/
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
#endif
#if 0
void FileManager::iterateDirectory(const std::filesystem::path& dirPath, int* itemCount)
{

    // Iterate through all the entries in the directory
    for (const auto& entry : std::filesystem::directory_iterator(dirPath))
    {
        // Check if the entry is a directory
        if (entry.is_directory())
        {
            std::cout << "Directory: " << entry.path().filename() << std::endl;
			*itemCount++;
            // Recursively call the function to iterate through the subdirectory
            //iterateDirectory(entry.path());
        }
        else
        {
            std::cout << "File: " << entry.path().filename() << std::endl;
            *itemCount++;
        }
    }
    std::cout<<"sub items : "<<*itemCount<<std::endl;

}
#endif

#if 0
const int FileManager::CheckSubItemCount(struct dirInfo const &info)
{
	std::cout << "CheckSubItemCount" << std::endl;
	int subItem_count=0;

	for (std::filesystem::directory_entry const& entry : std::filesystem::directory_iterator(info))
	{
		if(entry.path().has_stem()) subItem_count++;
	}

	return subItem_count;

}
#endif

void FileManager::RootPathList(std::filesystem::path const &currentpath,std::vector<std::filesystem::path>& rootPathList)
{
	std::filesystem::path dirEntry(currentpath);
	std::filesystem::path rootDir = dirEntry.root_path();
	std::filesystem::path parentPath = dirEntry.parent_path();

//	std::cout<<"Root path : "<<rootDir<<std::endl;

	while(dirEntry != rootDir)
	{
		rootPathList.push_back(dirEntry);
		dirEntry = parentPath;
		parentPath = dirEntry.parent_path();
//		std::cout<<"Path List : "<< dirEntry <<std::endl;
	}

}

void FileManager::PreviewSubItem(std::filesystem::path const &path,int* itemCount)
{
	std::cout << "PreviewSubItem" << std::endl;

    std::filesystem::directory_entry dirEntry(path);
    std::filesystem::directory_iterator dirIterator(dirEntry);

    for(auto const& dir_entry:  dirIterator)
	{
		if(dir_entry.is_directory())
		{
			//std::cout << "directory: " << dir_entry.path() << std::endl;
			(*itemCount)++;
		}
		else
		{
			//std::cout << "file: " << dir_entry.path() << std::endl;
			(*itemCount)++;
		}
	}

    std::cout<<"sub items : "<< (*itemCount) <<std::endl;

}

void FileManager::ReadDirectoryList(void)
{
	std::cout << "ReadDirectoryList" << std::endl;

    struct directory_content_info dirContentInfo = GetDirectoryInfo();
    int itemCount=0;
    std::filesystem::directory_entry dirEntry(std::filesystem::current_path());
    std::filesystem::directory_iterator dirIterator(dirEntry);
	RootPathList(dirEntry.path(),dirContentInfo.rootPathList);

    //std::cout<<"current path : "<<dirEntry.path()<<std::endl;
    for (auto const& dir_entry:  dirIterator)
    {
        if(dir_entry.is_directory())
        {

			PreviewSubItem(dir_entry.path(),&itemCount);
            	//std::cout << "directory: " << dir_entry.path() << std::endl;
            dirContentInfo.dirList.push_back({	dirEntry.path(),
            									dir_entry.path(),
            									dir_entry.path().string(),
            									dir_entry.path().filename().string(),
												"folder",
												itemCount,
												dir_entry.last_write_time(),
												"rwxrwxrwx"});

        }
        else
        {
            //std::cout << "file: " << dir_entry.path() << std::endl;
            dirContentInfo.fileList.push_back({dir_entry.path().string(),
            									dir_entry.path().filename().string(),
												dir_entry.path().extension().string(),
												std::to_string(dir_entry.file_size()),
												dir_entry.last_write_time(),
												"file",
												"root",
												"root",
												"rwxrwxrwx"});
        }
    }

    SetDirectoryInfo(dirContentInfo);

    this->m_dirContentInfo = dirContentInfo;

}


const struct FileManager::directory_content_info& FileManager::GetDirectoryInfo(void)
{
//	std::cout << "GetDirectoryInfo" << std::endl;

	return (this->m_dirContentInfo);
}

void FileManager::SetDirectoryInfo(struct FileManager::directory_content_info const &info)
{
    m_dirContentInfo = info;
}


/* 1. divide directory and files from input data.
   2. save each directory and files information. 
*/
void FileManager::DirectoryShowContent(void)
{
    std::cout << "DirectoryListing" << std::endl;
}

/*FileManager Remove Directory/Files*/
void FileManager::DirectoryRemove(struct FileManager::dirInfo const &info)
{
    std::cout << "m_DirectoryRemove" << std::endl;

    if(info.subPath.empty()) return;

    try{
        std::filesystem::remove_all(info.subPath);
    }
    catch(const std::exception& e)
    {
         std::cerr << "Error removing directory: " << e.what() << std::endl;
    }
    
}
void FileManager::FileRemove(struct FileManager::fileInfo const &info)
{
    std::cout << "FileRemove" << std::endl;

    if(info.path.empty()) return;

    std::filesystem::remove(info.path);
}

/*FileManager Rename Directory/Files*/
void FileManager::DirectoryRename(struct FileManager::dirInfo const &info)
{
    std::cout << "DirectoryRename" << std::endl;

    if(info.subPath.empty()) return;

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

void FileManager::FileRename(struct FileManager::fileInfo const &info)
{
    std::cout << "FileRename" << std::endl;

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

/*FileManager Create Directory/Files */
void FileManager::DirectoryCreate(void)
{
    std::cout << "DirectoryCreate" << std::endl;
    //todo
}

void FileManager::FileCreate(void)
{
    std::cout << "FileCreate" << std::endl;
    //todo
}

#if 0 // TODO implement later
void FileManager::DirectoryCopy(void) {}
void FileManager::DirectoryMove(void) {}
void FileManager::DirectorySearch(void) {}
void FileManager::DirectoryPrint(void) {}
void FileManager::DirectoryHelp(void) {}
void FileManager::DirectoryExit(void) {}
#endif
