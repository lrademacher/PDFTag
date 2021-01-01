#ifndef __PDF_FILE_H
#define __PDF_FILE_H

#include <string>
#include <vector>

class PdfFile {
public:
    PdfFile(std::string &path);

    std::string& getFilename();
    std::string& getCreationTime();
    std::vector<std::string>& getTags();
    bool containsTag(std::string &tag);
    bool containsTags(std::vector<std::string> &tags);
    void setTag(std::string &tag, bool selected);

    static std::vector<PdfFile>& getFiles();
    static PdfFile* getFileByFilename(std::string filename);

    static int loadPdfFilesFromDir(std::string &path);
    static std::vector<std::string>& getAllAvailableTags();
private:
    // empty constructor not allowed
    PdfFile();

    void readCreationTime(std::string &filepath);
    void readTags(std::string &filepath);

    static void updateAvailableTags();

    std::string mFilename;
    std::string mCreationTime;
    std::vector<std::string> mTags;

    // TODO: Make this dict: key=filename
    static std::vector<PdfFile> Files;

    static std::vector<std::string> AvailableTags;
};

#endif