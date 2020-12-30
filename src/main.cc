#include <gtk/gtk.h>
#include <stdio.h>

#define UI_FILE "PdfTag_GtkGui.glade"
#define GTK_CALLBACK extern "C" G_MODULE_EXPORT

#define XSTR(a) STR(a)
#define STR(a) #a
#define GET_WIDGET(name) data.name = GTK_WIDGET(gtk_builder_get_object(builder, XSTR(name)))

#define LOG_INFO (0)
#define LOG_WARN (1)
#define LOG_ERR  (2)
#define LOG(level, ...) printf((level==LOG_INFO) ? "INFO: " : (level==LOG_WARN) ? "WARNING: " : (level==LOG_ERR) ? "ERROR: " : ""); printf(__VA_ARGS__)


struct {
    GtkWidget *window;
    GtkWidget *file_chooser;
} data;

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
    GET_WIDGET(window);
    GET_WIDGET(file_chooser);

    g_signal_connect(data.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_builder_connect_signals(builder, NULL);

    /* Destroy builder, since we don't need it anymore */
    g_object_unref( G_OBJECT( builder ) );

    gtk_widget_show(data.window);

    gtk_main();

    return EXIT_SUCCESS;
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
on_file_chooser_ok_button_clicked(GtkButton *b)
{
    (void)b;

    LOG(LOG_INFO, "on_file_chooser_ok_button_clicked\n");

    // TODO: Do something 
    // TODO: Handle null
    LOG(LOG_INFO, "directory %s chosen\n", gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(data.file_chooser)));

    gtk_widget_hide(data.file_chooser);
}

GTK_CALLBACK void
on_file_chooser_cancel_button_clicked(GtkButton *b)
{
    (void)b;

    LOG(LOG_INFO, "on_file_chooser_cancel_button_clicked\n");

    gtk_widget_hide(data.file_chooser);
}
