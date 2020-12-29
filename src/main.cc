#include <gtk/gtk.h>

GtkWidget *window;

#define UI_FILE "PdfTag_GtkGui.glade"
#define GTK_CALLBACK extern "C" G_MODULE_EXPORT

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

    window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_builder_connect_signals(builder, NULL);

    /* Destroy builder, since we don't need it anymore */
    g_object_unref( G_OBJECT( builder ) );

    gtk_widget_show(window);

    gtk_main();

    return EXIT_SUCCESS;
}

GTK_CALLBACK void
on_FileQuit_activate(GtkMenuItem *m)
{
    (void)m;
    gtk_main_quit();
}