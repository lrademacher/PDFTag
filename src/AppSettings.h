#ifndef __APPSETTINGS_H
#define __APPSETTINGS_H

#include <string>

class AppSettings {
public:
    static bool getWorkingDirectory(std::string &dir);
    static void setWorkingDirectory(std::string &dir);
private:
    static const char* getHomeDir();
};

#endif