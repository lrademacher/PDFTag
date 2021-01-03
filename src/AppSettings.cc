#include "AppSettings.h"

#include <filesystem>
namespace fs = std::filesystem;

#include <string>
#include <iostream>
#include <fstream>

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

/* Defines/Macros */
#define SETTINGS_DIR "/.PDFTag/"
#define WDIR_FILE "wdir"
#define PDFEDIT_FILE "pdfeditor"

/* Implementation */

bool AppSettings::getWorkingDirectory(std::string &val)
{
    return getSetting(WDIR_FILE, val);
}

void AppSettings::setWorkingDirectory(std::string &val)
{
    setSetting(WDIR_FILE, val);
}

bool AppSettings::getPdfViewer(std::string &val)
{
    return getSetting(PDFEDIT_FILE, val);
}

void AppSettings::setPdfViewer(std::string &val)
{
    setSetting(PDFEDIT_FILE, val);
}

bool AppSettings::getSetting(const std::string &settingFile, std::string &settingVal)
{
    bool success = false;

    std::string wdirFilePath = getHomeDir();
    wdirFilePath += SETTINGS_DIR;
    wdirFilePath += settingFile;

    if (fs::exists(wdirFilePath))
    {
        std::ifstream ifs(wdirFilePath);

        ifs >> settingVal;
        success = true;
    }

    return success;
}

void AppSettings::setSetting(const std::string &settingFile, std::string &settingVal)
{
    std::string wdirFilePath = getHomeDir();
    wdirFilePath += SETTINGS_DIR;

    // ensure the directory exists
    if (!fs::exists(wdirFilePath))
    {
        fs::create_directory(wdirFilePath);
    }

    wdirFilePath += settingFile;

    std::ofstream ofs(wdirFilePath);

    ofs << settingVal;
}

const char *AppSettings::getHomeDir()
{
    struct passwd *pw = getpwuid(getuid());

    return pw->pw_dir;
}