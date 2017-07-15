#include <gtk/gtk.h>

GtkWidget *main_window		= NULL;
GtkWidget *setting_window	= NULL;
GtkWidget *help_window		= NULL;
GtkWidget *about_window		= NULL;
extern void setting_window_init(void);

void setting_window_delete_event()
{
	if (main_window == NULL || setting_window == NULL)
		return;

	gtk_widget_hide_all(setting_window);
	gtk_widget_show_all(main_window);
}

void setting_window_create_event()
{
	if (main_window == NULL || setting_window == NULL)
		return;

	setting_window_init();
	gtk_widget_hide_all(main_window);
	gtk_widget_show_all(setting_window);
}

void help_window_create_event()
{
	if (main_window == NULL || help_window == NULL)
		return;

	gtk_widget_hide_all(main_window);
	gtk_widget_show_all(help_window);
}

void help_window_delete_event()
{
	if (main_window == NULL || help_window == NULL)
		return;

	gtk_widget_hide_all(help_window);
	gtk_widget_show_all(main_window);
}

void about_window_create_event()
{
	if (help_window == NULL || about_window == NULL)
		return;

	gtk_widget_hide_all(help_window);
	gtk_widget_show_all(about_window);
}

void about_window_delete_event()
{
	if (help_window == NULL || about_window == NULL)
		return;

	gtk_widget_hide_all(about_window);
	gtk_widget_show_all(help_window);
}
