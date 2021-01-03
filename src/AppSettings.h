#ifndef __APPSETTINGS_H
#define __APPSETTINGS_H

#include <string>

class AppSettings {
public:
    static bool getWorkingDirectory(std::string &val);
    static void setWorkingDirectory(std::string &val);

    static bool getPdfViewer(std::string &val);
    static void setPdfViewer(std::string &val);
private:
    static bool getSetting(const std::string &settingFile, std::string &settingVal);
    static void setSetting(const std::string &settingFile, std::string &settingVal);
    static const char* getHomeDir();
};

#endif