/* Includes */
#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

/* Defines/Macros */

#define UI_FILE "PdfTag_GtkGui.glade"
#define GTK_CALLBACK extern "C" G_MODULE_EXPORT

#define XSTR(a) STR(a)
#define STR(a) #a
#define GET_GTK(name, type) data.name = type(gtk_builder_get_object(builder, XSTR(name)))

#define LOG_INFO (0)
#define LOG_WARN (1)
#define LOG_ERR  (2)
#define LOG(level, ...) printf((level==LOG_INFO) ? "INFO: " : (level==LOG_WARN) ? "WARNING: " : (level==LOG_ERR) ? "ERROR: " : ""); printf(__VA_ARGS__)

/* Prototypes */
static void
display_pdf_files_in_dir(gchar *directory);

/* Variables */
static struct 
{
    GtkWidget               *window;
    GtkWidget               *file_chooser;
    GtkWidget               *about_dialog;

    GtkTreeStore            *file_treestore;
    GtkTreeView             *file_treeview;
    GtkTreeViewColumn       *file_file_column;
    GtkTreeViewColumn       *file_date_column;
    GtkTreeSelection        *file_select;
    GtkCellRenderer         *file_file_renderer;
    GtkCellRenderer         *file_date_renderer;
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

    GET_GTK(file_treestore, GTK_TREE_STORE);
    GET_GTK(file_treeview, GTK_TREE_VIEW);
    GET_GTK(file_file_column, GTK_TREE_VIEW_COLUMN);
    GET_GTK(file_date_column, GTK_TREE_VIEW_COLUMN);
    GET_GTK(file_select, GTK_TREE_SELECTION);
    GET_GTK(file_file_renderer, GTK_CELL_RENDERER);
    GET_GTK(file_date_renderer, GTK_CELL_RENDERER);
    
    GET_GTK(tags_treestore, GTK_TREE_STORE);
    GET_GTK(tags_treeview, GTK_TREE_VIEW); 
    GET_GTK(tags_selected_column, GTK_TREE_VIEW_COLUMN);
    GET_GTK(tags_tag_column, GTK_TREE_VIEW_COLUMN);
    GET_GTK(tags_select, GTK_TREE_SELECTION);
    GET_GTK(tags_selected_renderer, GTK_CELL_RENDERER);
    GET_GTK(tags_tag_renderer, GTK_CELL_RENDERER);

    // attach renderers to columns
    gtk_tree_view_column_add_attribute(data.file_file_column, data.file_file_renderer, "text", 0);
    gtk_tree_view_column_add_attribute(data.file_date_column, data.file_date_renderer, "text", 1);
    
    gtk_tree_view_column_add_attribute(data.tags_selected_column, data.tags_selected_renderer, "boolean", 0);
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
getFileModifiedTime(char *file_path, char *modified_time_string, size_t max_str_size)
{
    struct stat attr;
    stat(file_path, &attr);
    //strcpy(modified_time_string, ctime(&attr.st_mtime));
    strftime(modified_time_string, max_str_size, "%F %T", localtime(&(attr.st_ctime)));
}

static void
display_pdf_files_in_dir(char *directory)
{
    FILE *fp;
    char find_result_line[1024];
    char cmd [300];
    char file_mod_str[20];

    strcpy(cmd, "/usr/bin/find ");
    strcat(cmd, directory);
    strcat(cmd, " -iname \"*.pdf\"");

    LOG(LOG_INFO, "executing command: %s\n", cmd);   

     /* Open the command for reading. */
    fp = popen(cmd, "r");
    if (fp == NULL) {
        printf("Failed to run command\n" );
        return;
    }

    // add data rows
    gtk_tree_store_clear(data.file_treestore);

    /* Read the output a line at a time - output it. */
    while (fgets(find_result_line, sizeof(find_result_line), fp) != NULL) {
        // remove trailing newline
        char *pos;
        if ((pos=strchr(find_result_line, '\n')) != NULL)
            *pos = '\0';

        getFileModifiedTime(find_result_line, file_mod_str, sizeof(file_mod_str));

        printf("%s", find_result_line);
        gtk_tree_store_append(data.file_treestore, &data.file_tree_iterator, NULL);
        gtk_tree_store_set(data.file_treestore, &data.file_tree_iterator, 0, basename(find_result_line), 1, file_mod_str, -1);
    }

    /* close */
    pclose(fp);
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

    display_pdf_files_in_dir(gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(data.file_chooser)));

    gtk_widget_hide(data.file_chooser);
}

GTK_CALLBACK void
on_file_chooser_cancel_button_clicked(GtkButton *b)
{
    (void)b;

    LOG(LOG_INFO, "on_file_chooser_cancel_button_clicked\n");

    gtk_widget_hide(data.file_chooser);
}
