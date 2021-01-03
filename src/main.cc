/* Includes */
#include <gtk/gtk.h>

#include "PdfFile.h"
#include "Logging.h"
#include "AppSettings.h"
#include "Util.h"

#include <filesystem>
namespace fs = std::filesystem;

// std::find
#include <algorithm>

#include <chrono> 
using namespace std::chrono; 

/* Defines/Macros */

#define UI_FILE "PdfTag_GtkGui.glade"
#define GTK_CALLBACK extern "C" G_MODULE_EXPORT

#define XSTR(a) STR(a)
#define STR(a) #a
#define GET_GTK(name, type) data.name = type(gtk_builder_get_object(builder, XSTR(name)))

#define TAGS_LABEL_STR "Tags of File: "

#define TAGSFILTER_UNTAGGED "<untagged>"

/* Types */
enum class CopyType {
    Copy,
    Move
};

/* Variables */
static struct
{
    GtkWidget *window;
    GtkWidget *file_chooser;
    GtkWidget *file_chooser_copy;
    GtkWidget *about_dialog;
    GtkWidget *loading_dialog;
    GtkWidget *settings_dialog;
    GtkWidget *error_dialog;
    GtkWidget *date_chooser_dialog;
    GtkWidget *selected_menu;

    GtkLabel *status_label;
    GtkLabel *file_table_label;

    GtkLabel *error_dialog_label;

    GtkEntry *new_tag_entry;

    GtkProgressBar *loading_progress_bar;
    GtkLabel *loading_dialog_label;

    GtkEntry *pdf_viewer_entry;

    GtkEntry *min_date_entry;
    GtkEntry *max_date_entry;
    GtkCalendar *date_chooser_calendar;

    GtkTreeStore *file_treestore;
    GtkTreeView *file_treeview;
    GtkTreeViewColumn *file_file_column;
    GtkTreeViewColumn *file_dir_column;
    GtkTreeViewColumn *file_date_column;
    GtkTreeViewColumn *file_tags_column;
    GtkTreeSelection *file_select;
    GtkCellRenderer *file_file_renderer;
    GtkCellRenderer *file_dir_renderer;
    GtkCellRenderer *file_date_renderer;
    GtkCellRenderer *file_tags_renderer;
    GtkTreeIter file_tree_iterator;

    GtkLabel *tagsearch_label;
    GtkTreeStore *tagsearch_treestore;
    GtkTreeView *tagsearch_treeview;
    GtkTreeViewColumn *tagsearch_selected_column;
    GtkTreeViewColumn *tagsearch_tag_column;
    GtkTreeSelection *tagsearch_select;
    GtkCellRenderer *tagsearch_selected_renderer;
    GtkCellRenderer *tagsearch_tag_renderer;
    GtkTreeIter tagsearch_tree_iterator;

    GtkLabel *tags_label;
    GtkTreeStore *tags_treestore;
    GtkTreeView *tags_treeview;
    GtkTreeViewColumn *tags_selected_column;
    GtkTreeViewColumn *tags_tag_column;
    GtkTreeSelection *tags_select;
    GtkCellRenderer *tags_selected_renderer;
    GtkCellRenderer *tags_tag_renderer;
    GtkTreeIter tags_tree_iterator;
} data;

static std::string searchTextStr;
static std::vector<std::string> filter;

static bool untaggedSelected = false;

static std::vector<PdfFile*> selectedFiles;
static GtkTreeIter selectedFileIter;

static GtkEntry* activeDateEntry = nullptr;

static CopyType copyType; 

/* Prototypes */
static void
loadFiles(std::string &path);

static void
display_files();

static void
display_tags(PdfFile &f, GtkTreeStore *tree_store, GtkTreeIter *iter);

static void
setup_tagfilter();

static void
update_tags_table();

static void
openSelectedPdf();

static void
copySelectedPdfTo(std::string &pathString, CopyType ct);

static void
raiseError(const std::string &errorString);

static void 
copy_selected_files(CopyType ct);

static void 
copy_selected_files_clipboard(CopyType ct);

/* Implementations */

int main(int argc, char **argv)
{
    GError *error = NULL;
    char uiFilePath[256];
    ssize_t uiFilePathLen = sizeof(uiFilePath);

    /* Init GTK+ */
    gtk_init(&argc, &argv);

    /* Prepare path to UI File*/
    int bytes = MIN(readlink("/proc/self/exe", uiFilePath, uiFilePathLen), uiFilePathLen - 1);
    if (bytes >= 0)
        uiFilePath[bytes] = '\0';

    char *lastChr = strrchr(uiFilePath, '/');
    lastChr += 1;
    *lastChr = '\0';
    strcat(uiFilePath, UI_FILE);

    // build GUI from glade file
    GtkBuilder *builder = gtk_builder_new();
    if (!gtk_builder_add_from_file(builder, uiFilePath, &error))
    {
        g_warning("%s", error->message);
        g_free(error);
        return (1);
    }

    // get widgets from builder
    GET_GTK(window, GTK_WIDGET);
    GET_GTK(file_chooser, GTK_WIDGET);
    GET_GTK(file_chooser_copy, GTK_WIDGET);
    GET_GTK(about_dialog, GTK_WIDGET);
    GET_GTK(loading_dialog, GTK_WIDGET);
    GET_GTK(settings_dialog, GTK_WIDGET);
    GET_GTK(error_dialog, GTK_WIDGET);
    GET_GTK(date_chooser_dialog, GTK_WIDGET);
    GET_GTK(selected_menu, GTK_WIDGET);

    GET_GTK(status_label, GTK_LABEL);
    GET_GTK(file_table_label, GTK_LABEL);

    GET_GTK(error_dialog_label, GTK_LABEL);

    GET_GTK(new_tag_entry, GTK_ENTRY);

    GET_GTK(loading_progress_bar, GTK_PROGRESS_BAR);
    GET_GTK(loading_dialog_label, GTK_LABEL);

    GET_GTK(pdf_viewer_entry, GTK_ENTRY);

    GET_GTK(min_date_entry, GTK_ENTRY);
    GET_GTK(max_date_entry, GTK_ENTRY);
    GET_GTK(date_chooser_calendar, GTK_CALENDAR);

    GET_GTK(file_treestore, GTK_TREE_STORE);
    GET_GTK(file_treeview, GTK_TREE_VIEW);
    GET_GTK(file_file_column, GTK_TREE_VIEW_COLUMN);
    GET_GTK(file_dir_column, GTK_TREE_VIEW_COLUMN);
    GET_GTK(file_date_column, GTK_TREE_VIEW_COLUMN);
    GET_GTK(file_tags_column, GTK_TREE_VIEW_COLUMN);
    GET_GTK(file_select, GTK_TREE_SELECTION);
    GET_GTK(file_file_renderer, GTK_CELL_RENDERER);
    GET_GTK(file_dir_renderer, GTK_CELL_RENDERER);
    GET_GTK(file_date_renderer, GTK_CELL_RENDERER);
    GET_GTK(file_tags_renderer, GTK_CELL_RENDERER);

    GET_GTK(tagsearch_label, GTK_LABEL);
    GET_GTK(tagsearch_treestore, GTK_TREE_STORE);
    GET_GTK(tagsearch_treeview, GTK_TREE_VIEW);
    GET_GTK(tagsearch_selected_column, GTK_TREE_VIEW_COLUMN);
    GET_GTK(tagsearch_tag_column, GTK_TREE_VIEW_COLUMN);
    GET_GTK(tagsearch_select, GTK_TREE_SELECTION);
    GET_GTK(tagsearch_selected_renderer, GTK_CELL_RENDERER);
    GET_GTK(tagsearch_tag_renderer, GTK_CELL_RENDERER);

    GET_GTK(tags_label, GTK_LABEL);
    GET_GTK(tags_treestore, GTK_TREE_STORE);
    GET_GTK(tags_treeview, GTK_TREE_VIEW);
    GET_GTK(tags_selected_column, GTK_TREE_VIEW_COLUMN);
    GET_GTK(tags_tag_column, GTK_TREE_VIEW_COLUMN);
    GET_GTK(tags_select, GTK_TREE_SELECTION);
    GET_GTK(tags_selected_renderer, GTK_CELL_RENDERER);
    GET_GTK(tags_tag_renderer, GTK_CELL_RENDERER);

    // attach renderers to columns
    gtk_tree_view_column_add_attribute(data.file_file_column, data.file_file_renderer, "text", 0);
    gtk_tree_view_column_add_attribute(data.file_dir_column, data.file_dir_renderer, "text", 1);
    gtk_tree_view_column_add_attribute(data.file_date_column, data.file_date_renderer, "text", 2);
    gtk_tree_view_column_add_attribute(data.file_tags_column, data.file_tags_renderer, "text", 3);

    gtk_tree_view_column_add_attribute(data.tags_selected_column, data.tags_selected_renderer, "active", 0);
    gtk_tree_view_column_add_attribute(data.tags_tag_column, data.tags_tag_renderer, "text", 1);

    gtk_tree_view_column_add_attribute(data.tagsearch_selected_column, data.tagsearch_selected_renderer, "active", 0);
    gtk_tree_view_column_add_attribute(data.tagsearch_tag_column, data.tagsearch_tag_renderer, "text", 1);

    g_signal_connect(data.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_builder_connect_signals(builder, NULL);

    /* Destroy builder, since we don't need it anymore */
    g_object_unref(G_OBJECT(builder));

    gtk_widget_show(data.window);

    // Load last-known working directory
    std::string workingDirectory;
    if (AppSettings::getWorkingDirectory(workingDirectory))
    {
        loadFiles(workingDirectory);
    }

    gtk_main();

    return EXIT_SUCCESS;
}

static void
display_tags(PdfFile &f, GtkTreeStore *tree_store, GtkTreeIter *iter)
{
    std::vector<std::string> &tags = f.getTags();

    std::string tagString = "";
    auto it = tags.begin();

    while (it != tags.end())
    {
        tagString.append(*it++);
        if (it != tags.end())
            tagString.append(",");
    }

    gtk_tree_store_set(tree_store, iter, 3, tagString.c_str(), -1);
}

static void
display_files()
{
    // clear table
    gtk_tree_store_clear(data.file_treestore);

    int itemsShown = 0;

    for (PdfFile f : PdfFile::getFiles())
    {
        // Tag-based filtering
        if (untaggedSelected)
        {
            if (!f.getTags().empty())
                continue; // skip for all non-empty tags-lists
        }
        else
        {
            if (!filter.empty() && !(f.containsTags(filter)))
                continue; // skip if filter tags are not contained
        }

        // String-based filtering
        if (!searchTextStr.empty())
        {
            std::string currentFilename = fs::path(f.getFilename()).filename().string();
            Util::tolower(currentFilename);
            if (currentFilename.find(searchTextStr) == std::string::npos)
                continue; // skip if filter string is not contained
        }

        // date-based filtering
        std::string minDate = gtk_entry_get_text(data.min_date_entry);
        std::string maxDate = gtk_entry_get_text(data.max_date_entry);
        if(!minDate.empty())
        {
            int compareLen = MIN(minDate.size(), f.getCreationTime().size());
            if(minDate.substr(0,compareLen).compare(f.getCreationTime().substr(0,compareLen)) > 0)
                continue; // skip if the min-date is newer than the date of the file
        }
        if(!maxDate.empty())
        {
            int compareLen = MIN(maxDate.size(), f.getCreationTime().size());
            if(maxDate.substr(0,compareLen).compare(f.getCreationTime().substr(0,compareLen)) < 0)
                continue; // skip if the maxdate is older than the date of the file
        }

        ++itemsShown;

        gtk_tree_store_append(data.file_treestore, &data.file_tree_iterator, NULL);

        gtk_tree_store_set(data.file_treestore, &data.file_tree_iterator, 0, fs::path(f.getFilename()).filename().c_str(), 1, fs::path(f.getFilename()).parent_path().c_str(), 2, f.getCreationTime().c_str(), -1);

        display_tags(f, data.file_treestore, &data.file_tree_iterator);
    }

    std::string tableLabelText = "File table (";
    tableLabelText += std::to_string(itemsShown);
    tableLabelText += " items shown)";

    gtk_label_set_text(data.file_table_label, tableLabelText.c_str());
}

static void
update_tags_table()
{
    // clear table
    gtk_tree_store_clear(data.tags_treestore);

    for (std::string tag : PdfFile::getAllAvailableTags())
    {
        gtk_tree_store_append(data.tags_treestore, &data.tags_tree_iterator, NULL);

        bool containsTag = false;
        for (PdfFile* f: selectedFiles)
        {
            /* Check if at least one of the selected files contains the currently looked at tag */
            if (f->containsTag(tag))
            {
                containsTag = true;
            }
        }

        gtk_tree_store_set(data.tags_treestore, &data.tags_tree_iterator, 0, containsTag, 1, tag.c_str(), -1);
    }
}

static void
setup_tagfilter()
{
    // clear filter
    filter.clear();

    // clear table
    gtk_tree_store_clear(data.tagsearch_treestore);

    // populate table
    for (std::string tag : PdfFile::getAllAvailableTags())
    {
        gtk_tree_store_append(data.tagsearch_treestore, &data.tagsearch_tree_iterator, NULL);
        gtk_tree_store_set(data.tagsearch_treestore, &data.tagsearch_tree_iterator, 0, FALSE, 1, tag.c_str(), -1);
    }

    gtk_tree_store_append(data.tagsearch_treestore, &data.tagsearch_tree_iterator, NULL);
    gtk_tree_store_set(data.tagsearch_treestore, &data.tagsearch_tree_iterator, 0, FALSE, 1, TAGSFILTER_UNTAGGED, -1);
}

static void
loadFiles(std::string &path)
{
    bool loadingOkay;
    bool loadingCompleted = false;
    float loadingCompleteFraction = 0.0f;

    auto start = high_resolution_clock::now(); 

    gtk_widget_show(data.loading_dialog);

    loadingOkay = PdfFile::beginLoadPdfFilesFromDir(path);

    std::string dialogStr = "Loading PDF files from directory: ";
    dialogStr += path;

    gtk_label_set_text(data.loading_dialog_label, dialogStr.c_str());

    if (loadingOkay)
    {
        do
        {
            loadingCompleted = PdfFile::loadPdfFilesFromDirIncrement(loadingCompleteFraction);

            gtk_progress_bar_set_fraction(data.loading_progress_bar, loadingCompleteFraction);

            /* --- Repaint any windows - like the progress bar --- */
            while (gtk_events_pending())
            {
                gtk_main_iteration();
            }
        } while (!loadingCompleted);
    }

    setup_tagfilter();
    update_tags_table();
    display_files();

    auto stop = high_resolution_clock::now(); 
    LOG(LOG_INFO, "loaded %d files in %dms\n", (int)PdfFile::getFiles().size(), (int)duration_cast<milliseconds>(stop - start).count());

    std::string statusString = "Loaded " + std::to_string(PdfFile::getFiles().size()) + " files from root directory: " + path;

    gtk_label_set_label(data.status_label, statusString.c_str());

    gtk_widget_hide(data.loading_dialog);
}

static void
openSelectedPdf()
{
    std::string pdfViewer;

    if (!AppSettings::getPdfViewer(pdfViewer) || pdfViewer.empty())
    {
        throw "PdfViewer not defined in settings.";
    }

    if (selectedFiles.empty())
    {
        throw "No file selected.";
    }

    for (PdfFile* selectedFile: selectedFiles)
    {
        std::string cmdStr = pdfViewer;

        cmdStr += " \"";
        cmdStr += selectedFile->getFilename();
        cmdStr += "\" &";

        LOG(LOG_INFO, "executing command %s\n", cmdStr.c_str());

        // TODO: Check if pdf-viewer is available

        system(cmdStr.c_str());
    }
}

static void
copySelectedPdfTo(std::string &pathString, CopyType ct)
{
    for (PdfFile* selectedFile : selectedFiles)
    {
        std::string cmdStr;

        if (pathString.empty())
            return;

        if (CopyType::Copy == ct) {
            cmdStr += "cp \"";
        } else if (CopyType::Move == ct) {
            cmdStr += "mv \"";
        } else {
            LOG(LOG_ERR, "Unknown copy type %d\n", (int)ct);
            return;
        }
        cmdStr += selectedFile->getFilename();
        cmdStr += "\" \"";
        cmdStr += pathString;
        cmdStr += "\"";

        LOG(LOG_INFO, "executing command %s\n", cmdStr.c_str());

        system(cmdStr.c_str());
    }
}

static void
raiseError(const std::string &errorString)
{
    std::string displayString = "Error: " + errorString;

    gtk_label_set_text(data.error_dialog_label, displayString.c_str());

    gtk_widget_show(data.error_dialog);
}

static void 
copy_selected_files(CopyType ct)
{
    std::string dir;

    if (selectedFiles.empty())
    {
        raiseError("No file selected.");
        return;
    }

    copyType = ct;

    if (AppSettings::getWorkingDirectory(dir))
    {
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(data.file_chooser_copy), dir.c_str());
    }

    gtk_widget_show(data.file_chooser_copy);
}

static void 
copy_selected_files_clipboard(CopyType ct)
{
    if (selectedFiles.empty())
    {
        raiseError("No file selected.");
        return;
    }

    std::string cmdStr;
    std::string clipboardContent;

    if (CopyType::Copy == ct)
    {
        clipboardContent = "copy";
    }
    else
    {
        clipboardContent = "cut";
    }

    for (PdfFile* selectedFile : selectedFiles)
    {        
        clipboardContent += "\nfile://";
        clipboardContent += selectedFile->getFilename();
    }

    cmdStr = "echo -n \"";
    cmdStr += clipboardContent;
    cmdStr += "\" | xclip -i -selection clipboard -t x-special/gnome-copied-files";

    system(cmdStr.c_str());
}

/* ************* GTK SIGNAL HANDLERS ******************* */

GTK_CALLBACK void
on_FileOpen_activate(GtkMenuItem *m)
{
    std::string dir;

    (void)m;

    LOG(LOG_INFO, "on_FileOpen_activate\n");

    if (AppSettings::getWorkingDirectory(dir))
    {
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(data.file_chooser), dir.c_str());
    }

    gtk_widget_show(data.file_chooser);
}

GTK_CALLBACK void
on_open_selected_activate(GtkMenuItem *m)
{
    (void)m;

    LOG(LOG_INFO, "on_open_selected_activate\n");

    try
    {
        openSelectedPdf();
    }
    catch (const char *msg)
    {
        raiseError(msg);
    }
}

GTK_CALLBACK void
on_save_selected_activate(GtkMenuItem *m)
{
    (void)m;

    LOG(LOG_INFO, "on_copy_selected_activate\n");

    copy_selected_files(CopyType::Copy);
}

GTK_CALLBACK void
on_move_selected_activate(GtkMenuItem *m)
{
    (void)m;

    LOG(LOG_INFO, "on_move_selected_activate\n");

    copy_selected_files(CopyType::Move);
}

GTK_CALLBACK void
on_copy_clipboard_selected_activate(GtkMenuItem *m)
{
    (void)m;

    LOG(LOG_INFO, "on_copy_clipboard_selected_activate\n");

    copy_selected_files_clipboard(CopyType::Copy);
}

GTK_CALLBACK void
on_cut_clipboard_selected_activate(GtkMenuItem *m)
{
    (void)m;

    LOG(LOG_INFO, "on_copy_clipboard_selected_activate\n");

    copy_selected_files_clipboard(CopyType::Move);
}

GTK_CALLBACK void
on_FileQuit_activate(GtkMenuItem *m)
{
    (void)m;

    LOG(LOG_INFO, "on_FileQuit_activate\n");

    gtk_main_quit();
}

GTK_CALLBACK void
on_preferences_activate(GtkMenuItem *m)
{
    (void)m;

    LOG(LOG_INFO, "on_preferences_activate\n");

    std::string pdfViewer;
    if (!AppSettings::getPdfViewer(pdfViewer))
    {
        pdfViewer = "";
    }
    gtk_entry_set_text(data.pdf_viewer_entry, pdfViewer.c_str());

    gtk_widget_show(data.settings_dialog);
}

GTK_CALLBACK void
on_HelpAbout_activate(GtkMenuItem *m)
{
    (void)m;

    LOG(LOG_INFO, "on_HelpAbout_activate\n");

    gtk_widget_show(data.about_dialog);
}

GTK_CALLBACK void
on_about_dialog_response(GtkDialog *dialog, gint response_id)
{
    (void)dialog;
    (void)response_id;

    LOG(LOG_INFO, "on_about_dialog_response\n");

    gtk_widget_hide(data.about_dialog);
}

GTK_CALLBACK gboolean
on_window_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    (void)widget;
    (void)data;

    LOG(LOG_INFO, "on_window_key_press_event\n");

    if (event->state & GDK_CONTROL_MASK)
    {
        if (event->keyval == GDK_KEY_c){
            LOG(LOG_INFO, "CTRL + C\n");
            copy_selected_files_clipboard(CopyType::Copy);
            return TRUE;
        }
        else if (event->keyval == GDK_KEY_x){
            LOG(LOG_INFO, "CTRL + X\n");
            copy_selected_files_clipboard(CopyType::Move);
            return TRUE;
        }
    }

    return FALSE;
}

/* Detect table double-click */
GTK_CALLBACK gboolean
on_file_treeview_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
    (void)widget;
    (void)user_data;

    if (event->type == GDK_DOUBLE_BUTTON_PRESS)
    {
        LOG(LOG_INFO, "double-click on table detected\n");

        try
        {
            openSelectedPdf();
        }
        catch (const char *msg)
        {
            raiseError(msg);
        }
    }

    /* pass event to underlying handlers. */
    return FALSE;
}

GTK_CALLBACK void
on_file_chooser_ok_button_clicked(GtkButton *b)
{
    (void)b;

    LOG(LOG_INFO, "on_file_chooser_ok_button_clicked\n");

    LOG(LOG_INFO, "directory %s chosen\n", gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(data.file_chooser)));

    std::string pathString(gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(data.file_chooser)));

    loadFiles(pathString);

    AppSettings::setWorkingDirectory(pathString);

    gtk_widget_hide(data.file_chooser);
}

GTK_CALLBACK void
on_file_chooser_cancel_button_clicked(GtkButton *b)
{
    (void)b;

    LOG(LOG_INFO, "on_file_chooser_cancel_button_clicked\n");

    gtk_widget_hide(data.file_chooser);
}

GTK_CALLBACK void
on_file_chooser_copy_cancel_button_clicked(GtkButton *b)
{
    (void)b;

    LOG(LOG_INFO, "on_file_chooser_copy_cancel_button_clicked\n");

    gtk_widget_hide(data.file_chooser_copy);
}

GTK_CALLBACK void
on_file_chooser_copy_ok_button_clicked(GtkButton *b)
{
    (void)b;

    LOG(LOG_INFO, "on_file_chooser_copy_ok_button_clicked\n");

    std::string pathString(gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(data.file_chooser_copy)));
    LOG(LOG_INFO, "directory %s chosen\n", pathString.c_str());

    copySelectedPdfTo(pathString, copyType);

    gtk_widget_hide(data.file_chooser_copy);
}

GTK_CALLBACK void
on_settings_save_clicked(GtkButton *b)
{
    (void)b;

    LOG(LOG_INFO, "on_settings_save_clicked\n");

    std::string pdfEditor = gtk_entry_get_text(data.pdf_viewer_entry);

    if (!pdfEditor.empty())
    {
        AppSettings::setPdfViewer(pdfEditor);
    }

    gtk_widget_hide(data.settings_dialog);
}

GTK_CALLBACK void
on_settings_cancel_clicked(GtkButton *b)
{
    (void)b;

    LOG(LOG_INFO, "on_settings_cancel_clicked\n");

    gtk_widget_hide(data.settings_dialog);
}

GTK_CALLBACK void
on_error_dialog_close_button_clicked(GtkButton *b)
{
    (void)b;

    LOG(LOG_INFO, "on_error_dialog_close_button_clicked\n");

    gtk_widget_hide(data.error_dialog);
}

static void addSelectedFile(GtkTreeModel *model,
		  GtkTreePath *path,
		  GtkTreeIter *iter,
		  gpointer data)
{
    gchar *selected_filename;
    gchar *selected_dir;
    std::string fullfilename;
    (void)path;
    (void)data;

    gtk_tree_model_get (model, iter, 0, &selected_filename, 1, &selected_dir, -1);

    fullfilename = selected_dir;
    fullfilename += "/";
    fullfilename += selected_filename;
    LOG(LOG_INFO, "col 0 = %s\n", fullfilename.c_str());

    // update selected file
    selectedFiles.push_back(PdfFile::getFileByFilename(fullfilename)); 
}

GTK_CALLBACK void
on_file_selection_changed(GtkWidget *w)
{
    (void)w;

    LOG(LOG_INFO, "on_file_selection_changed\n");

    std::string tagsLabel = TAGS_LABEL_STR;

    gint numSelectedEntries = gtk_tree_selection_count_selected_rows(GTK_TREE_SELECTION(w));

    selectedFiles.clear();

    if (numSelectedEntries == 0)
    {
        tagsLabel += "-";
        
        LOG(LOG_INFO, "No file selected\n");

        gtk_widget_set_sensitive(data.selected_menu, false);
    }
    else
    {
        gtk_tree_selection_selected_foreach (GTK_TREE_SELECTION(w), addSelectedFile, NULL);  

        tagsLabel += selectedFiles[0]->getFilename();
        if (selectedFiles.size() > 1)
        {
            tagsLabel += " + " + std::to_string(selectedFiles.size() - 1) + " files.";
        }
         
        gtk_widget_set_sensitive(data.selected_menu, true);
    }

    gtk_label_set_label(data.tags_label, tagsLabel.c_str());

    // populate tags table
    update_tags_table();
}

GTK_CALLBACK void
on_tagsearch_selected_renderer_toggled(GtkCellRendererToggle *cell, gchar *path_string)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    gboolean selected = FALSE;
    char *tag_name;

    (void)cell;

    LOG(LOG_INFO, "on_tagsearch_selected_renderer_toggled\n");
    LOG(LOG_INFO, "signal path: %s\n", path_string);

    model = gtk_tree_view_get_model(data.tagsearch_treeview);
    gtk_tree_model_get_iter_from_string(model, &iter, path_string);

    gtk_tree_model_get(model, &iter, 0, &selected, 1, &tag_name, -1);

    LOG(LOG_INFO, "row text: %s selected: %d\n", tag_name, selected);

    if (0 == strcmp(tag_name, TAGSFILTER_UNTAGGED))
    {
        selected = !selected;
        untaggedSelected = selected;

        if (untaggedSelected)
        {
            // clear tag filter list as untagged is a special filter which does not allow tag-filtering
            filter.clear();

            GtkTreeIter iter;
            bool valid = gtk_tree_model_get_iter_first(model, &iter);

            while (valid)
            {
                gtk_tree_store_set(data.tagsearch_treestore, &iter, 0, FALSE, -1);
                valid = gtk_tree_model_iter_next(model, &iter);
            }
        }
    }
    else
    {
        // In case untagged is selected, other tags must not be selected
        if (untaggedSelected)
            return;

        selected = !selected;

        // update filter
        if (!selected)
        {
            auto find_res = std::find(filter.begin(), filter.end(), tag_name);
            if (find_res != filter.end())
                filter.erase(find_res);
        }
        else
        {
            filter.push_back(tag_name);
        }
    }

    // store new toggle setting
    gtk_tree_store_set(data.tagsearch_treestore, &iter, 0, selected, -1);

    // display files
    display_files();
}

GTK_CALLBACK void
on_tags_selected_renderer_toggled(GtkCellRendererToggle *cell, gchar *path_string)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    gboolean selected = FALSE;
    char *tag_name;

    (void)cell;

    LOG(LOG_INFO, "on_tags_selected_renderer_toggled\n");
    LOG(LOG_INFO, "signal path: %s\n", path_string);

    model = gtk_tree_view_get_model(data.tags_treeview);
    gtk_tree_model_get_iter_from_string(model, &iter, path_string);

    gtk_tree_model_get(model, &iter, 0, &selected, 1, &tag_name, -1);

    LOG(LOG_INFO, "row text: %s selected: %d\n", tag_name, selected);

    selected = !selected;

    for (PdfFile* selectedFile : selectedFiles)
    {
        std::string tagStr = tag_name;
        selectedFile->setTag(tagStr, (bool)selected);
    }

    update_tags_table();

    // update file-list as it also contains the tags of all files
    for (PdfFile* selectedFile: selectedFiles)
    {
        display_tags(*selectedFile, data.file_treestore, &selectedFileIter);
    }
}

GTK_CALLBACK void
on_new_tag_button_clicked(GtkButton *b)
{
    (void)b;
    LOG(LOG_INFO, "on_new_tag_button_clicked\n");

    std::string tagStr = gtk_entry_get_text(data.new_tag_entry);

    if (tagStr.empty())
    {
        return; // empty tag not allowed
    }

    if (!selectedFiles.empty())
    {
        for (PdfFile* selectedFile: selectedFiles)
        {
            selectedFile->setTag(tagStr, TRUE);
        }
    }

    // update tag view
    update_tags_table();

    // update file-list as it also contains the tags of all files
    for (PdfFile* selectedFile: selectedFiles)
    {
        display_tags(*selectedFile, data.file_treestore, &selectedFileIter);
    }
}

GTK_CALLBACK void
on_file_search_search_changed(GtkSearchEntry *e)
{
    const char *searchText = gtk_entry_get_text(GTK_ENTRY(e));
    searchTextStr = searchText;
    Util::tolower(searchTextStr);

    // display files
    display_files();
}

GTK_CALLBACK void
on_date_entry_icon_press(GtkEntry *entry,
                             GtkEntryIconPosition icon_pos,
                             GdkEvent *event,
                             gpointer user_data)
{
    (void)event;
    (void)user_data;

    LOG(LOG_INFO, "on_date_entry_icon_press entry:%s pos:%d\n", (entry == data.min_date_entry) ? "min" : (entry == data.max_date_entry) ? "max" : "?", icon_pos);

    if(GTK_ENTRY_ICON_PRIMARY == icon_pos) 
    {
        activeDateEntry = entry;
        std::string dateString = gtk_entry_get_text(entry); 
        if (!dateString.empty())
        {
            guint year, month, day;
            bool yearFound = false, monthFound = false, dayFound = false;
            std::stringstream dateStringStream(dateString); 
            std::string temp;
            if(std::getline(dateStringStream, temp,':')) {
                if(std::stringstream(temp)>>year)
                {
                    yearFound = true;
                }
            }
            if(std::getline(dateStringStream, temp,':')) {
                if(std::stringstream(temp)>>month)
                {
                    monthFound = true;
                }
            }
            if(std::getline(dateStringStream, temp,':')) {
                if(std::stringstream(temp)>>day)
                {
                    dayFound = true;
                }
            }
            if(yearFound && monthFound)
                gtk_calendar_select_month(data.date_chooser_calendar, month - 1, year);
            if(dayFound)
                gtk_calendar_select_day(data.date_chooser_calendar, day);
        }
        gtk_widget_show(data.date_chooser_dialog);
    } 
    else if(GTK_ENTRY_ICON_SECONDARY == icon_pos) 
    {
        // clear
        gtk_entry_set_text(entry, "");
    }
}

GTK_CALLBACK void
on_date_entry_changed(GtkEditable *editable)
{
    (void)editable;

    LOG(LOG_INFO, "on_date_entry_changed\n");

    if(strlen(gtk_entry_get_text(GTK_ENTRY(editable))) > 0)
    {
        gtk_entry_set_icon_sensitive(GTK_ENTRY(editable), GTK_ENTRY_ICON_SECONDARY, TRUE);
    }
    else
    {
        gtk_entry_set_icon_sensitive(GTK_ENTRY(editable), GTK_ENTRY_ICON_SECONDARY, FALSE);
    }

    // display files (as filter condition was updated)
    display_files();
}

GTK_CALLBACK void
on_date_chooser_cancel_clicked(GtkButton *button)
{
    (void)button;

    gtk_widget_hide(data.date_chooser_dialog);

    LOG(LOG_INFO, "on_date_chooser_cancel_clicked\n");
}

GTK_CALLBACK void
on_date_chooser_ok_clicked(GtkButton *button)
{
    (void)button;

    guint year, month, day;
    char datestr[4+1+2+1+2+1];

    gtk_calendar_get_date(data.date_chooser_calendar, &year, &month, &day);

    sprintf(datestr, "%04d:%02d:%02d", year, month + 1, day);

    gtk_entry_set_text(activeDateEntry, datestr);

    gtk_widget_hide(data.date_chooser_dialog);

    LOG(LOG_INFO, "on_date_chooser_ok_clicked\n");
}

GTK_CALLBACK gboolean
hide_instead_of_delete(GtkWidget *widget)
{
    LOG(LOG_INFO, "hide_instead_of_delete\n");

    gtk_widget_hide(widget);
    return TRUE;
}

GTK_CALLBACK gboolean
do_not_delete(GtkWidget *widget)
{
    (void)widget;

    LOG(LOG_INFO, "do_not_delete\n");

    return TRUE;
}