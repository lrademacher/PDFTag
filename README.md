# PDFTag
PDF tag (keyword) management desktop application

# Dependencies
libimage-exiftool-perl for PDF metadata modification
This software uses std::filesystem, so a gcc with C++17 support shall be used.

# TODO
## Table handling
* Don't lose selection on filter change as long as the selected item is available in the current filter
* Open all selected PDFs
* Copy all selected PDFs in a directory
## Filtering
* Update Tag-filter list in case AvailableTags changes (new tag added, tag removed from all files)
* String-based filter
* Date-based filter
* Add special filter <untagged>
* Improve filtering: Can we just deactivate entries in table?
* Sorted tag list
* search query in tag list
## Other
* Integrate into deb package management to resolve dependencies and make installation easy
* Port to gtkmm

# Issues
* Big file lists (~270 PDFs) tags filtering => Stack overflow