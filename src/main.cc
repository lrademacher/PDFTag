/* Includes */
#include <gtk/gtk.h>

#include "PdfFile.h"
#include "Logging.h"

#include <filesystem>
namespace fs = std::filesystem;

// std::find
#include <algorithm>

/* Defines/Macros */

#define UI_FILE "PdfTag_GtkGui.glade"
#define GTK_CALLBACK extern "C" G_MODULE_EXPORT

#define XSTR(a) STR(a)
#define STR(a) #a
#define GET_GTK(name, type) data.name = type(gtk_builder_get_object(builder, XSTR(name)))

#define TAGS_LABEL_STR "Tags of File: "

/* Types */

/* Variables */
static struct 
{
    GtkWidget               *window;
    GtkWidget               *file_chooser;
    GtkWidget               *about_dialog;
    GtkLabel                *status_label;
    GtkEntry                *new_tag_entry;

    GtkTreeStore            *file_treestore;
    GtkTreeView             *file_treeview;
    GtkTreeViewColumn       *file_file_column;
    GtkTreeViewColumn       *file_dir_column;
    GtkTreeViewColumn       *file_date_column;
    GtkTreeViewColumn       *file_tags_column;
    GtkTreeSelection        *file_select;
    GtkCellRenderer         *file_file_renderer;
    GtkCellRenderer         *file_dir_renderer;
    GtkCellRenderer         *file_date_renderer;
    GtkCellRenderer         *file_tags_renderer;
    GtkTreeIter             file_tree_iterator;

    GtkLabel                *tagsearch_label;
    GtkTreeStore            *tagsearch_treestore;
    GtkTreeView             *tagsearch_treeview; 
    GtkTreeViewColumn       *tagsearch_selected_column;
    GtkTreeViewColumn       *tagsearch_tag_column;
    GtkTreeSelection        *tagsearch_select;
    GtkCellRenderer         *tagsearch_selected_renderer;
    GtkCellRenderer         *tagsearch_tag_renderer;
    GtkTreeIter             tagsearch_tree_iterator;

    GtkLabel                *tags_label;
    GtkTreeStore            *tags_treestore;
    GtkTreeView             *tags_treeview; 
    GtkTreeViewColumn       *tags_selected_column;
    GtkTreeViewColumn       *tags_tag_column;
    GtkTreeSelection        *tags_select;
    GtkCellRenderer         *tags_selected_renderer;
    GtkCellRenderer         *tags_tag_renderer;
    GtkTreeIter             tags_tree_iterator;
} data;

std::vector<std::string> filter;

PdfFile *selectedFile = nullptr;
GtkTreeIter selectedFileIter;
GtkTreeModel *selectedFileModel;

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

/* Implementations */

int
main(int argc, char **argv) {
    GError *error = NULL;
    char uiFilePath[256];
    ssize_t uiFilePathLen = sizeof(uiFilePath); 

    /* Init GTK+ */
    gtk_init(&argc, &argv);

    /* Prepare path to UI File*/
    int bytes = MIN(readlink("/proc/self/exe", uiFilePath, uiFilePathLen), uiFilePathLen - 1);
    if(bytes >= 0)
        uiFilePath[bytes] = '\0';

    char * lastChr = strrchr(uiFilePath, '/');
    lastChr += 1; 
    *lastChr = '\0';
    strcat(uiFilePath, UI_FILE);

    // build GUI from glade file
    GtkBuilder *builder = gtk_builder_new();
    if( ! gtk_builder_add_from_file( builder, uiFilePath, &error ) )
    {
        g_warning( "%s", error->message );
        g_free( error );
        return( 1 );
    }

    // get widgets from builder
    GET_GTK(window, GTK_WIDGET);
    GET_GTK(file_chooser, GTK_WIDGET);
    GET_GTK(about_dialog, GTK_WIDGET);
    
    GET_GTK(status_label, GTK_LABEL);

    GET_GTK(new_tag_entry, GTK_ENTRY);

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
    g_object_unref( G_OBJECT( builder ) );

    gtk_widget_show(data.window);

    // TODO: persistency
    std::string default_path = "/home/lars/Nextcloud.Webo/Dokumente/Scans";
    loadFiles(default_path);

    gtk_main();

    return EXIT_SUCCESS;
}

static void
display_tags(PdfFile &f, GtkTreeStore *tree_store, GtkTreeIter *iter)
{
    std::vector<std::string>& tags = f.getTags();

    std::string tagString = "";
    auto it = tags.begin();

    while(it != tags.end())
    {
        tagString.append(*it++);
        if(it != tags.end())
            tagString.append(",");
    }

    gtk_tree_store_set(tree_store, iter, 3, tagString.c_str(), -1);
}

static void
display_files()
{
    // clear table
    gtk_tree_store_clear(data.file_treestore);

    for (PdfFile f: PdfFile::getFiles())
    {
        if(!filter.empty() && !(f.containsTags(filter)))
            continue; // skip if filter tags are not contained

        gtk_tree_store_append(data.file_treestore, &data.file_tree_iterator, NULL);
        
        gtk_tree_store_set(data.file_treestore, &data.file_tree_iterator, 0, fs::path( f.getFilename() ).filename().c_str(), 1, fs::path( f.getFilename() ).parent_path().c_str(), 2, f.getCreationTime().c_str(), -1);
        
        display_tags(f, data.file_treestore, &data.file_tree_iterator);
    }
}

static void
update_tags_table()
{
    // clear table
    gtk_tree_store_clear(data.tags_treestore);

    if(nullptr != selectedFile)
    {
        for(std::string tag : PdfFile::getAllAvailableTags())
        {
            gtk_tree_store_append(data.tags_treestore, &data.tags_tree_iterator, NULL);
            gtk_tree_store_set(data.tags_treestore, &data.tags_tree_iterator, 0, selectedFile->containsTag(tag), 1, tag.c_str(), -1);
        }
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
    for(std::string tag : PdfFile::getAllAvailableTags())
    {
        gtk_tree_store_append(data.tagsearch_treestore, &data.tagsearch_tree_iterator, NULL);
        gtk_tree_store_set(data.tagsearch_treestore, &data.tagsearch_tree_iterator, 0, FALSE, 1, tag.c_str(), -1);
    }
}

GTK_CALLBACK void
on_FileOpen_activate(GtkMenuItem *m)
{
    (void)m;

    LOG(LOG_INFO, "on_FileOpen_activate\n");   

    gtk_widget_show(data.file_chooser);
}

GTK_CALLBACK void
on_FileQuit_activate(GtkMenuItem *m)
{
    (void)m;

    LOG(LOG_INFO, "on_FileQuit_activate\n");

    gtk_main_quit();
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

static void
loadFiles(std::string &path)
{
    int numFiles = PdfFile::loadPdfFilesFromDir(path);
    display_files();
    setup_tagfilter();

    std::string statusString = "Loaded " + std::to_string(numFiles) + " files from root directory: " + path;

    gtk_label_set_label(data.status_label, statusString.c_str());
}

GTK_CALLBACK void
on_file_chooser_ok_button_clicked(GtkButton *b)
{
    (void)b;

    LOG(LOG_INFO, "on_file_chooser_ok_button_clicked\n");

    LOG(LOG_INFO, "directory %s chosen\n", gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(data.file_chooser)));

    std::string pathString (gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(data.file_chooser)));

    loadFiles(pathString);

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
on_file_selection_changed(GtkWidget *w)
{
    (void)w;
    gchar *selected_filename;
    gchar *selected_dir;
    char tagsLabel[100];

    std::string fullfilename;

    LOG(LOG_INFO, "on_file_selection_changed\n");

    if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(w), &selectedFileModel, &selectedFileIter) == FALSE)
        return;

    gtk_tree_model_get(selectedFileModel, &selectedFileIter, 0, &selected_filename, 1, &selected_dir, -1);

    fullfilename = selected_dir;
    fullfilename += "/";
    fullfilename += selected_filename;
    LOG(LOG_INFO, "col 0 = %s\n", fullfilename.c_str());

    strcpy(tagsLabel, TAGS_LABEL_STR);
    strcat(tagsLabel, selected_filename);
    gtk_label_set_label(data.tags_label, tagsLabel);

    // update selected file
    selectedFile = PdfFile::getFileByFilename(fullfilename);

    // populate tags table
    update_tags_table();
}

GTK_CALLBACK void
on_tagsearch_selected_renderer_toggled (GtkCellRendererToggle *cell, gchar *path_string) {
    GtkTreeIter iter;
    GtkTreeModel *model;
    gboolean selected = FALSE;
    char *tag_name;
    
    (void)cell;

    LOG(LOG_INFO, "on_tagsearch_selected_renderer_toggled\n");
    LOG(LOG_INFO, "signal path: %s\n", path_string);

    model = gtk_tree_view_get_model(data.tagsearch_treeview);
    gtk_tree_model_get_iter_from_string (model, &iter, path_string);

    gtk_tree_model_get(model, &iter, 0, &selected, 1, &tag_name, -1);

    LOG(LOG_INFO, "row text: %s selected: %d\n", tag_name, selected);

    selected = !selected;

    // update filter
    if(!selected)
    {
        auto find_res = std::find(filter.begin(), filter.end(), tag_name);
        if(find_res != filter.end())
            filter.erase(find_res);
    }
    else
    {
        filter.push_back(tag_name);
    }

    // display files
    display_files();

    // store new toggle setting
    gtk_tree_store_set(data.tagsearch_treestore, &iter, 0, selected, -1);
}

GTK_CALLBACK void
on_tags_selected_renderer_toggled (GtkCellRendererToggle *cell, gchar *path_string) {
    GtkTreeIter iter;
    GtkTreeModel *model;
    gboolean selected = FALSE;
    char *tag_name;
    
    (void)cell;

    LOG(LOG_INFO, "on_tags_selected_renderer_toggled\n");
    LOG(LOG_INFO, "signal path: %s\n", path_string);

    model = gtk_tree_view_get_model(data.tags_treeview);
    gtk_tree_model_get_iter_from_string (model, &iter, path_string);

    gtk_tree_model_get(model, &iter, 0, &selected, 1, &tag_name, -1);

    LOG(LOG_INFO, "row text: %s selected: %d\n", tag_name, selected);

    selected = !selected;

    if(nullptr != selectedFile)
    {
        std::string tagStr = tag_name;
        selectedFile->setTag(tagStr, (bool)selected);
    }

    update_tags_table();

    // update file-list as it also contains the tags of all files
    if(nullptr != selectedFile)
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

    if(tagStr.empty())
    {
        return; // empty tag not allowed
    }

    if(nullptr != selectedFile)
    {
        selectedFile->setTag(tagStr, TRUE);
    }

    // update tag view
    update_tags_table();

    // update file-list as it also contains the tags of all files
    if(nullptr != selectedFile)
    {
        display_tags(*selectedFile, data.file_treestore, &selectedFileIter);
    }
}