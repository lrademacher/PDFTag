#include <gtk/gtk.h>

GtkBuilder *builder;
GtkWidget *window;

int
main(int argc, char **argv) {
    gtk_init(&argc, &argv);

    // build GUI from glade file
    builder = gtk_builder_new_from_file("PdfTag_GtkGui.glade");    

    window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show(window);

    gtk_main();

    return EXIT_SUCCESS;

    /*app = gtk_application_new("org.gtk.example",
        G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate",
        G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return (status);*/
}

static void
activate(GtkApplication *app,
    gpointer user_data) {
    GtkWidget *window;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Hello GNOME");
    gtk_widget_show_all(window);
}

static void
on_FileMenu_activate(GtkMenuItem *menuItem)
{
    if(gtk_menu_item_get_label(menuItem) == "Quit")
    {
        gtk_main_quit();
    }
}