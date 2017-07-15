#include <gtk/gtk.h>
#include <libgen.h>
#include "create_main_window.h"
#include "create_setting_window.h"
#include "create_help_window.h"
#include "create_about_window.h"
#include "manage_window_show_hide.h"
#include "thread.h"
#include "params.h"

GdkPixbuf *create_pixbuf (const gchar *filename)
{
       GdkPixbuf *pixbuf;
       GError *error = NULL;
       pixbuf = gdk_pixbuf_new_from_file(filename, &error);
       if(!pixbuf)
       {
               fprintf(stderr, "%s\n", error->message);
               g_error_free(error);
       }
       else
               ;
}

int main(int argc, char *argv[])
{

	if (setuid(0) != 0) {
		DSPRINT("Error: Cannot obtain root privilege, please try \"sudo %s\"\n",
			basename(argv[0]));
		return 0;
	}

	/* Initialize GTK */
	g_log_set_handler("Gtk", G_LOG_LEVEL_WARNING, (GLogFunc)gtk_false, NULL);
	gtk_init(&argc, &argv);
	g_log_set_handler("Gtk", G_LOG_LEVEL_WARNING, g_log_default_handler, NULL);

	restoreParameters();

	create_background_thread();

	main_window = create_main_window();
	setting_window = create_setting_window();
	help_window = create_help_window();
	about_window = create_about_window();

	gtk_window_set_icon(GTK_WINDOW(main_window), create_pixbuf("/usr/local/sbin/DS.ico"));
	gtk_window_set_icon(GTK_WINDOW(setting_window), create_pixbuf("/usr/local/sbin/DS.ico"));
	gtk_window_set_icon(GTK_WINDOW(help_window), create_pixbuf("/usr/local/sbin/DS.ico"));
	gtk_window_set_icon(GTK_WINDOW(about_window), create_pixbuf("/usr/local/sbin/DS.ico"));

	g_signal_connect(G_OBJECT(setting_window), "delete_event",
				G_CALLBACK(setting_window_delete_event), NULL);
	g_signal_connect(G_OBJECT(help_window), "delete_event",
				G_CALLBACK(help_window_delete_event), NULL);
	g_signal_connect(G_OBJECT(about_window), "delete_event",
				G_CALLBACK(about_window_delete_event), NULL);
	setting_window_delete_event();

	main_window_text_change();
	setting_window_text_change();
	help_window_text_change();

	/* Start to GTK MAIN loop */
	gtk_main();

	return 0;
}
