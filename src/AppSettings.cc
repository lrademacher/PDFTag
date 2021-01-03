#include "AppSettings.h"

#include <filesystem>
namespace fs = std::filesystem;

#include <string>
#include <iostream>
#include <fstream>

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#define SETTINGS_DIR "/.PDFTag/"
#define WDIR_FILE "wdir"

bool AppSettings::getWorkingDirectory(std::string &dir)
{
    bool success = false;

    std::string wdirFilePath = getHomeDir();
    wdirFilePath += SETTINGS_DIR;
    wdirFilePath += WDIR_FILE;

    if ( fs::exists(wdirFilePath) )
    {
        std::ifstream ifs(wdirFilePath);
        
        ifs >> dir;
        success = true;
    }

    return success;
}

void AppSettings::setWorkingDirectory(std::string &dir)
{
    std::string wdirFilePath = getHomeDir();
    wdirFilePath += SETTINGS_DIR;

    // ensure the directory exists:
    if ( !fs::exists(wdirFilePath) )
    {
        fs::create_directory(wdirFilePath);
    }
    
    wdirFilePath += WDIR_FILE;

    std::ofstream ofs(wdirFilePath);

    ofs << dir;
}

const char* AppSettings::getHomeDir()
{
    struct passwd *pw = getpwuid(getuid());

    return pw->pw_dir;
}