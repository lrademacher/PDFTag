# PDFTag
PDF tag (keyword) management desktop application

# Dependencies
libimage-exiftool-perl for PDF metadata modification
This software uses std::filesystem, so a gcc with C++17 support shall be used.

# TODO
* Update Tag-filter list in case AvailableTags changes (new tag added, tag removed from all files)
* Don't lose selection on filter change as long as the selected item is available in the current filter
* Open PDF
* Copy all selected PDFs in a directory
* String-based filter
* Date-based filter
* Add special filter <untagged>
* Integrate into deb package management to resolve dependencies and make installation easy
* Add persistency of working directory
* Improve filtering: Can we just deactivate entries in table?
* Port to gtkmm

# Issues
* Big file lists (~270 PDFs) tags filtering => Stack overflow