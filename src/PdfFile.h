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

    static int loadPdfFilesFromDir(std::string &path, std::vector<PdfFile> &files);
private:
    // empty constructor not allowed
    PdfFile();

    void readCreationTime(std::string &filepath);
    void readTags(std::string &filepath);

    std::string mFilename;
    std::string mCreationTime;
    std::vector<std::string> mTags;
};

#endif