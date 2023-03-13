/* Includes */
#include "PdfFile.h"
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <string.h>
#include <algorithm>

#include "Logging.h"
#include "Util.h"

#include <stdexcept>
#include <sstream> 

#include <regex>

/* Defines/Macros */
#define EXIF_TOOL_BIN "exiftool "
#define EXIF_TOOL_READ_TAGS_FLAG "-Keywords "
#define EXIF_TOOL_READ_CREATEDATE_FLAG "-createdate "
#define EXIF_TOOL_READ_None_FLAG "-None "
#define EXIF_TOOL_PDF_FILTER "-ext PDF "
#define EXIF_TOOL_RECURSIVE_DIR_SEARCH "-r "
#define EXIF_TOOL_ADDTAGS_STR "exiftool -Keywords+="
#define EXIF_TOOL_REMOVETAGS_STR "exiftool -Keywords-="

/* Variables */
std::vector<std::string> PdfFile::AvailableTags;
std::vector<PdfFile> PdfFile::Files;
FILE *PdfFile::loadingFp;
int PdfFile::numFilesToLoad;

std::string PdfFile::currentlyLoadingFilename;
std::string PdfFile::currentlyLoadingCreateDate;
std::vector<std::string> PdfFile::currentlyLoadingTags;

static std::regex regexFilename ("======== (.*)");
static std::regex regexKeywords ("Keywords[ ]*: (.*)");
static std::regex regexCreateDate ("Create Date[ ]*: (.*)");

/* Implementation */
PdfFile::PdfFile(std::string &path, std::string &creationDate, std::vector<std::string> &tags)
{
    mFilename = path;
    mCreationDate = creationDate;
    mTags = tags;
}

std::string& PdfFile::getFilename()
{
    return mFilename;
}

std::string& PdfFile::getCreationTime()
{
    return mCreationDate;
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
    cmd = EXIF_TOOL_BIN;
    cmd += EXIF_TOOL_READ_None_FLAG;
    cmd += EXIF_TOOL_PDF_FILTER;
    cmd += EXIF_TOOL_RECURSIVE_DIR_SEARCH;
    cmd += path;

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

    // Clear temporary string storage
    currentlyLoadingFilename.clear();
    currentlyLoadingCreateDate.clear();
    currentlyLoadingTags.clear();

    // assemble command
    cmd = EXIF_TOOL_BIN;
    cmd += EXIF_TOOL_READ_TAGS_FLAG;
    cmd += EXIF_TOOL_READ_CREATEDATE_FLAG;
    cmd += EXIF_TOOL_PDF_FILTER;
    cmd += EXIF_TOOL_RECURSIVE_DIR_SEARCH;
    cmd += path;

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

        std::string lineContent (find_result_line);
        
        bool regexFound = testRegex(regexFilename, lineContent);

        if(regexFound)
        {   
            // new entry => push last entry to files vector if available
            if(!currentlyLoadingFilename.empty())
                Files.push_back(PdfFile(currentlyLoadingFilename, currentlyLoadingCreateDate, currentlyLoadingTags));

            // setup filename of next entry and clear other fields
            currentlyLoadingFilename = lineContent;
            currentlyLoadingCreateDate.clear();
            currentlyLoadingTags.clear();
        }
        else
        {
            regexFound = testRegex(regexKeywords, lineContent);
            if(regexFound)
            {
                LOG(LOG_INFO, "Splitting: %s\n", lineContent.c_str()); 
                Util::splitString(lineContent, currentlyLoadingTags, ", ");
                for (std::string s: currentlyLoadingTags)
                {
                    LOG(LOG_INFO, "%s\n", s.c_str()); 
                }
            }
            else
            {
                regexFound = testRegex(regexCreateDate, lineContent);
                if(regexFound)
                {
                    currentlyLoadingCreateDate = lineContent;
                }
            }
            
        }
    }
    else
    {
        complete = true;

        // last entry => push gathered info to files vector
        Files.push_back(PdfFile(currentlyLoadingFilename, currentlyLoadingCreateDate, currentlyLoadingTags));
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


bool PdfFile::testRegex(std::regex &r, std::string &s)
{
    bool found = false;

    std::smatch m;
    
    if(regex_match(s, m, r))
    {
        if (m.size() == 2) {
            std::ssub_match base_sub_match = m[1];
            s = base_sub_match;
            found = true;
        }    
    }

    return found;
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