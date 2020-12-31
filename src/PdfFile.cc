/* Includes */
#include "PdfFile.h"
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <string.h>

#include "Logging.h"

/* Defines/Macros */
#define EXIF_TOOL_READ_STR "exiftool -Keywords "

#define FIND_COMMAND "/usr/bin/find "
#define FIND_EXPRESSION_PDF " -iname \"*.pdf\""

/* Implementation */
PdfFile::PdfFile(std::string &path)
{
    mFilename = path;
    readCreationTime(path);
    readTags(path);
}

std::string& PdfFile::getFilename()
{
    return mFilename;
}

std::string& PdfFile::getCreationTime()
{
    return mCreationTime;
}

std::vector<std::string>& PdfFile::getTags()
{
    return mTags;
}

void PdfFile::readCreationTime(std::string &filepath)
{
    char modified_time_string[20];

    struct stat attr;
    stat(filepath.c_str(), &attr);
    strftime(modified_time_string, sizeof(modified_time_string), "%F %T", localtime(&(attr.st_ctime)));

    mCreationTime = modified_time_string;
}

void PdfFile::readTags(std::string &filepath)
{
    FILE *fp;
    char exiftool_result_line[1024];
    std::string cmd;
    
    // assemble command
    cmd = EXIF_TOOL_READ_STR;
    cmd += "\"";
    cmd += filepath;
    cmd += "\"";

    LOG(LOG_INFO, "executing command: %s\n", cmd.c_str());   

    // Open the command for reading.
    fp = popen(cmd.c_str(), "r");
    if (fp == NULL) {
        LOG(LOG_ERR, "Failed to run command\n");
    }

    // Read the output a line at a time - output it.
    while (fgets(exiftool_result_line, sizeof(exiftool_result_line), fp) != NULL) {
        // remove trailing newline
        char *pos;
        if ((pos=strchr(exiftool_result_line, '\n')) != NULL)
            *pos = '\0';

        char delim[] = ", ";

        if ((pos=strstr(exiftool_result_line, ": ")) != NULL) // Found keywords
        {
            pos += 2;

            // split by comma
            char *ptr = strtok(pos, delim);

            while(ptr != NULL)
            {
                LOG(LOG_INFO, "found tag: %s\n", ptr);
                std::string tag_str = ptr;
                mTags.push_back(tag_str);
                ptr = strtok(NULL, delim);
            }
        }
    }
}

int PdfFile::loadPdfFilesFromDir(std::string &path, std::vector<PdfFile> &files)
{
    FILE *fp;
    char find_result_line[1024];
    std::string cmd;

    // clear old filelist
    files.clear();

    // assemble command
    cmd = FIND_COMMAND;
    cmd += path;
    cmd += FIND_EXPRESSION_PDF;

    LOG(LOG_INFO, "executing command: %s\n", cmd.c_str());   

    // Open the command for reading.
    fp = popen(cmd.c_str(), "r");
    if (fp == NULL) {
        printf("Failed to run command\n" );
        return 0;
    }

    // Read the output a line at a time - output it.
    while (fgets(find_result_line, sizeof(find_result_line), fp) != NULL) {
        // remove trailing newline
        char *pos;
        if ((pos=strchr(find_result_line, '\n')) != NULL)
            *pos = '\0';

        std::string filepath (find_result_line);

        files.push_back(filepath);
    }

    LOG(LOG_INFO, "found %d files\n", (int)files.size());   

    pclose(fp);

    return (int)files.size();
}