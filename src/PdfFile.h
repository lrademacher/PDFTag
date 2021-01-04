#ifndef __PDF_FILE_H
#define __PDF_FILE_H

#include <string>
#include <vector>
#include <regex>

class PdfFile {
public:
    PdfFile(std::string &path, std::string &creationDate, std::vector<std::string> &tags);

    std::string& getFilename();
    std::string& getCreationTime();
    std::vector<std::string>& getTags();
    bool containsTag(std::string &tag);
    bool containsTags(std::vector<std::string> &tags);
    void setTag(std::string &tag, bool selected);

    static std::vector<PdfFile>& getFiles();
    static PdfFile* getFileByFilename(std::string filename);

    static bool beginLoadPdfFilesFromDir(std::string &path);
    static bool loadPdfFilesFromDirIncrement(float &loadingCompleteFraction);

    static std::vector<std::string>& getAllAvailableTags();
private:
    // empty constructor not allowed
    PdfFile();

    void readCreationTime(std::string &filepath);
    void readTags(std::string &filepath);

    static void updateAvailableTags();
    static int getNumPdfInDir(std::string &path);
    static bool testRegex(std::regex &r, std::string &s);

    std::string mFilename;
    std::string mCreationDate;
    std::vector<std::string> mTags;

    // TODO: Make this dict: key=filename
    static std::vector<PdfFile> Files;

    static std::vector<std::string> AvailableTags;

    static FILE *loadingFp;
    static int numFilesToLoad;
    static std::string currentlyLoadingFilename;
    static std::string currentlyLoadingCreateDate;
    static std::vector<std::string> currentlyLoadingTags;
};

#endif