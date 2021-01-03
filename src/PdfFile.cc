/* Includes */
#include "PdfFile.h"
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <string.h>
#include <algorithm>

#include "Logging.h"

#include <stdexcept>
#include <sstream> 

/* Defines/Macros */
#define EXIF_TOOL_READTAGS_STR "exiftool -Keywords "
#define EXIF_TOOL_ADDTAGS_STR "exiftool -Keywords+="
#define EXIF_TOOL_REMOVETAGS_STR "exiftool -Keywords-="
#define EXIF_TOOL_READCREATIONDATE_STR "exiftool -createdate "

#define FIND_COMMAND "/usr/bin/find "
#define FIND_EXPRESSION_PDF " -iname \"*.pdf\""
#define FIND_COUNT_EXTENSION " | wc -l"

/* Variables */
std::vector<std::string> PdfFile::AvailableTags;
std::vector<PdfFile> PdfFile::Files;
FILE *PdfFile::loadingFp;
int PdfFile::numFilesToLoad;

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

bool PdfFile::containsTag(std::string &tag)
{
    return (std::find(mTags.begin(), mTags.end(), tag) != mTags.end());
}

bool PdfFile::containsTags(std::vector<std::string> &tags)
{
    bool containsTags = true;
    
    for (std::string tag : tags)
    {
        if(!containsTag(tag))
        {
            containsTags = false;
        }
    }

    return containsTags;
}

void PdfFile::setTag(std::string &tag, bool selected)
{
    FILE *fp;
    char exiftool_result_line[1024];
    std::string cmd;
    
    LOG(LOG_INFO, "setting tag: %s to %d\n", tag.c_str(), selected);   

    // assemble command
    if(selected)
    {
        cmd = EXIF_TOOL_ADDTAGS_STR;
    }
    else
    {
        cmd = EXIF_TOOL_REMOVETAGS_STR;
    }
    
    cmd += tag;

    cmd += " \"";
    cmd += mFilename;
    cmd += "\"";

    LOG(LOG_INFO, "executing command: %s\n", cmd.c_str());   

    // Open the command for reading.
    fp = popen(cmd.c_str(), "r");
    if (fp == NULL) {
        LOG(LOG_ERR, "Failed to run command\n");
    }

    // Read the output a line at a time - output it.
    while (fgets(exiftool_result_line, sizeof(exiftool_result_line), fp) != NULL) {
        if(strncmp(exiftool_result_line, "    1 image files updated", 25) != 0)
        {
            LOG(LOG_ERR, "Commando did not execute as expected: %s.\n", exiftool_result_line);
        }
    }

    // update tags structure
    if(selected)
    {
        mTags.push_back(tag);
    }
    else
    {
        auto find_res = std::find(mTags.begin(), mTags.end(), tag);
        if(find_res != mTags.end())
            mTags.erase(find_res);
    }

    // Update overall available tag list
    updateAvailableTags();
}

void PdfFile::readTags(std::string &filepath)
{
    FILE *fp;
    char exiftool_result_line[1024];
    std::string cmd;
    
    // assemble command
    cmd = EXIF_TOOL_READTAGS_STR;
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
    if (fgets(exiftool_result_line, sizeof(exiftool_result_line), fp) != NULL) {
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

void PdfFile::readCreationTime(std::string &filepath)
{
    FILE *fp;
    char exiftool_result_line[1024];
    std::string cmd;
    
    // assemble command
    cmd = EXIF_TOOL_READCREATIONDATE_STR;
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
    if (fgets(exiftool_result_line, sizeof(exiftool_result_line), fp) != NULL) {
        // remove trailing newline
        char *pos;
        if ((pos=strchr(exiftool_result_line, '\n')) != NULL)
            *pos = '\0';

        if ((pos=strstr(exiftool_result_line, ": ")) != NULL) // Found date
        {
            pos += 2;

            char *posPlus;
            if((posPlus=strstr(exiftool_result_line, "+")) != NULL) // Found +
            {
                // strip all after '+' sign
                *posPlus = '\0';
            }

            mCreationTime = pos;
        }
    }
}

/* Static */

std::vector<PdfFile>& PdfFile::getFiles()
{
    return Files;
}

PdfFile* PdfFile::getFileByFilename(std::string filename)
{
    std::vector<PdfFile>::iterator it = std::find_if(Files.begin(), Files.end(), [filename](PdfFile &obj){
        return obj.getFilename() == filename;
    } );
    if(it != Files.end())
    {
        return &(*it);
    }
    
    return nullptr;
}

int PdfFile::getNumPdfInDir(std::string &path)
{
    FILE *fp;
    std::string cmd;
    char result_line[1024];
    int numPdf;

    // assemble command
    cmd = FIND_COMMAND;
    cmd += path;
    cmd += FIND_EXPRESSION_PDF;
    cmd += FIND_COUNT_EXTENSION;

    LOG(LOG_INFO, "executing command: %s\n", cmd.c_str());   

    // Open the command for reading.
    fp = popen(cmd.c_str(), "r");
    if (fp == NULL) {
        printf("Failed to run command\n" );
        return 0;
    }

    // Read the output a line at a time - output it.
    while (fgets(result_line, sizeof(result_line), fp) != NULL) {
        // Do nothing. We are just interested in the last line
    }

    std::string numPdfStr = result_line;
    std::stringstream numPdfStrStream(numPdfStr); 

    numPdfStrStream >> numPdf;

    return numPdf;
}

bool PdfFile::beginLoadPdfFilesFromDir(std::string &path)
{
    std::string cmd;

    // clear old filelist
    Files.clear();

    numFilesToLoad = getNumPdfInDir(path);

    // assemble command
    cmd = FIND_COMMAND;
    cmd += path;
    cmd += FIND_EXPRESSION_PDF;

    LOG(LOG_INFO, "executing command: %s\n", cmd.c_str());   

    // Open the command for reading.
    loadingFp = popen(cmd.c_str(), "r");
    if (loadingFp == NULL) {
        printf("Failed to run command\n" );
        return false;
    }
    return true;
}

bool PdfFile::loadPdfFilesFromDirIncrement(float &loadingCompleteFraction)
{
    bool complete = false;

    char find_result_line[1024];
    
    int numFilesLoaded;
   

    // Read the output a line at a time - output it.
    if (fgets(find_result_line, sizeof(find_result_line), loadingFp) != NULL) {
        // remove trailing newline
        char *pos;
        if ((pos=strchr(find_result_line, '\n')) != NULL)
            *pos = '\0';

        std::string filepath (find_result_line);

        Files.push_back(filepath);
    }
    else
    {
        complete = true;
    }

    numFilesLoaded = (int)Files.size();    

    LOG(LOG_INFO, "Loaded %d of %d files\n", numFilesLoaded, numFilesToLoad);   

    loadingCompleteFraction = (float)numFilesLoaded / (float)numFilesToLoad;

    if(complete)
    {   
        /* finalize */
        pclose(loadingFp);

        updateAvailableTags();
    }

    return complete;
}

void PdfFile::updateAvailableTags()
{
    AvailableTags.clear();

    for (PdfFile pf : Files)
    {
        for (std::string tag : pf.getTags())
        {
            if (std::find(AvailableTags.begin(), AvailableTags.end(), tag) == AvailableTags.end())
            {
                AvailableTags.push_back(tag);
            }
        }
    }
}

std::vector<std::string>& PdfFile::getAllAvailableTags()
{
    return AvailableTags;
}