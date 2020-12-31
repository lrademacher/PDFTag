/* Includes */
#include <gtk/gtk.h>

#include "PdfFile.h"
#include "Logging.h"

#include <filesystem>
namespace fs = std::filesystem;

/* Defines/Macros */

#define UI_FILE "PdfTag_GtkGui.glade"
#define GTK_CALLBACK extern "C" G_MODULE_EXPORT

#define XSTR(a) STR(a)
#define STR(a) #a
#define GET_GTK(name, type) data.name = type(gtk_builder_get_object(builder, XSTR(name)))

/* Types */

/* Variables */
static struct 
{
    GtkWidget               *window;
    GtkWidget               *file_chooser;
    GtkWidget               *about_dialog;
    GtkLabel                *status_label;

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

    GtkTreeStore            *tags_treestore;
    GtkTreeView             *tags_treeview; 
    GtkTreeViewColumn       *tags_selected_column;
    GtkTreeViewColumn       *tags_tag_column;
    GtkTreeSelection        *tags_select;
    GtkCellRenderer         *tags_selected_renderer;
    GtkCellRenderer         *tags_tag_renderer;
    GtkTreeIter             tags_tree_iterator;
} data;

static std::vector<PdfFile> files;

/* Prototypes */


/* Implementations */

int
main(int argc, char **argv) {
    GError *error = NULL;

    /* Init GTK+ */
    gtk_init(&argc, &argv);

    // build GUI from glade file
    GtkBuilder *builder = gtk_builder_new();
    if( ! gtk_builder_add_from_file( builder, UI_FILE, &error ) )
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
    
    gtk_tree_view_column_add_attribute(data.tags_tag_column, data.tags_tag_renderer, "text", 1);

    g_signal_connect(data.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_builder_connect_signals(builder, NULL);

    /* Destroy builder, since we don't need it anymore */
    g_object_unref( G_OBJECT( builder ) );

    gtk_widget_show(data.window);

    gtk_main();

    return EXIT_SUCCESS;
}

static void
display_files()
{
    // clear table
    gtk_tree_store_clear(data.file_treestore);

    for (PdfFile f: files)
    {
        gtk_tree_store_append(data.file_treestore, &data.file_tree_iterator, NULL);

        std::vector<std::string>& tags = f.getTags();

        std::string tagString = "";
        auto it = tags.begin();

        while(it != tags.end())
        {
            tagString.append(*it++);
            if(it != tags.end())
                tagString.append(",");
        }
        
        gtk_tree_store_set(data.file_treestore, &data.file_tree_iterator, 0, fs::path( f.getFilename() ).filename().c_str(), 1, fs::path( f.getFilename() ).parent_path().c_str(), 2, f.getCreationTime().c_str(), 3, tagString.c_str(), -1);
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

GTK_CALLBACK void
on_file_chooser_ok_button_clicked(GtkButton *b)
{
    (void)b;

    LOG(LOG_INFO, "on_file_chooser_ok_button_clicked\n");

    LOG(LOG_INFO, "directory %s chosen\n", gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(data.file_chooser)));

    std::string pathString (gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(data.file_chooser)));

    int numFiles = PdfFile::loadPdfFilesFromDir(pathString, files);
    display_files();

    std::string statusString = "Loaded " + std::to_string(numFiles) + " files.";

    gtk_label_set_label(data.status_label, statusString.c_str());

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
    gchar *value;
    GtkTreeIter iter;
    GtkTreeModel *model;

    LOG(LOG_INFO, "on_file_selection_changed\n");

    if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(w), &model, &iter) == FALSE)
        return;

    gtk_tree_model_get(model, &iter, 0, &value, -1);
    LOG(LOG_INFO, "col 0 = %s\n", value);

    // gtk_tree_model_get(model, &iter, 1, &value, -1);
}