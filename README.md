# PDFTag
A GTK3-based PDF tag (keyword) management desktop application

![](res/Screenshot.png)

# Dependencies
* libimage-exiftool-perl for PDF metadata reading/modifying
* This software uses std::filesystem, so a gcc with C++17 support shall be used.
* cp

# Features
* List of all PDFs contained in the selected working directory
* Display of PDF Keywords (Tags) and PDF creation date
* Filtering via Tags
* Filtering via string (Filename)
* Filtering via Min/Max Date
* Opening files (via configurable PDF viewer)
* Saving files to destination directory
