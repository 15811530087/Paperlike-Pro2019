#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include "thread.h"
#include "manage_window_show_hide.h"

GtkWidget *window_help;
GtkWidget *checking_update_button;
GtkWidget *about_button;
GtkWidget *Language_label;
GtkWidget *Language_select;
GtkWidget *OK_button;
GtkWidget *Cancel_button;
GtkWidget *Combo_button;
GtkWidget *Link_button;

extern void saveParameters(void);

void help_window_text_change()
{
	if (language == LANG_ZH) {
		gtk_button_set_label(GTK_BUTTON(checking_update_button), HELP_UI_UPDATE_CH);
		gtk_button_set_label(GTK_BUTTON(about_button), HELP_UI_ABOUT_CH);
	} else {
		gtk_button_set_label(GTK_BUTTON(checking_update_button), HELP_UI_UPDATE_EN);
		gtk_button_set_label(GTK_BUTTON(about_button), HELP_UI_ABOUT_EN);
	}
}

static void checking_update_button_action(GtkWidget *widget, gpointer data)
{
	/*
	gtk_moz_embed_set_profile_path("/home/qwq/zhihui","mybrowser");
	gtk_moz_embed_set_path("/usr/lib/xulrunner-1.9.2.24");
	GtkWidget *html =gtk_moz_embed_new(); 
	gtk_moz_embed_load_url(GTK_MOZ_EMBED(html),"http://www.126.com/");
	*/
}

static void about_button_action(GtkWidget *widget, gpointer data)
{
	about_window_create_event();
	DSPRINT("%s.\n", __func__);
}

static void Cancel_action(GtkWidget *widget, gpointer data)
{
	help_window_delete_event();
	DSPRINT("%s.\n", __func__);
}

static void OK_action(GtkWidget *widget, gpointer data)
{
	help_window_delete_event();

	if (gtk_combo_box_get_active(GTK_COMBO_BOX(Combo_button)) == 0)
		language = LANG_ZH;
	else
		language = LANG_EN;

	main_window_text_change();
	setting_window_text_change();
	help_window_text_change();
	saveParameters();
	gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);
}

GtkWidget *create_help_window()
{
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *fixed;

	window_help = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_resizable(GTK_WINDOW(window_help), FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(window_help), 30);
	gtk_window_set_position(GTK_WINDOW(window_help), GTK_WIN_POS_CENTER);

	vbox = gtk_vbox_new(FALSE, 20);

	/* Checking update button */
	fixed = gtk_fixed_new();
	checking_update_button = gtk_link_button_new_with_label("http://www.dasung.com/", "checking update");
	//gtk_link_button_set_visited(GTK_LINK_BUTTON(Link_button), TRUE);
	//gtk_fixed_put(GTK_FIXED(fixed), checking_update_button, 0, 0);
	//checking_update_button = gtk_button_new_with_label("Checking new version");
	gtk_widget_set_size_request(checking_update_button, 180, 30);
	gtk_widget_show(checking_update_button);
	gtk_fixed_put(GTK_FIXED(fixed), checking_update_button, 0, 0);
	//gtk_widget_set_size_request(checking_update_button, 80, 20);
	//gtk_box_pack_start(GTK_BOX(vbox), checking_update_button, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), fixed, FALSE, FALSE, 0);

	/* Language setting label */
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_set_spacing(GTK_BOX(hbox), 25);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	Language_label = gtk_label_new("语言切换/Language");
	gtk_widget_set_size_request(Language_label, 180, 30);
	//gtk_widget_set_size_request(Language_label, 80, 20);
	gtk_box_pack_start(GTK_BOX(hbox), Language_label, FALSE, FALSE, 0);
	Combo_button = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(Combo_button), "中文");
	gtk_combo_box_append_text(GTK_COMBO_BOX(Combo_button), "English");
	if (language == LANG_ZH)
		gtk_combo_box_set_active(GTK_COMBO_BOX(Combo_button), 0);
	else
		gtk_combo_box_set_active(GTK_COMBO_BOX(Combo_button), 1);
	gtk_box_pack_start(GTK_BOX(hbox), Combo_button, FALSE, FALSE, 0);

	/* About button */
	fixed = gtk_fixed_new();
	about_button = gtk_button_new_with_label("About");
	gtk_widget_set_size_request(about_button, 180, 30);
	gtk_widget_show(about_button);
	gtk_fixed_put(GTK_FIXED(fixed), about_button, 0, 0);
	//gtk_widget_set_size_request(about_button, 80, 20);
	gtk_box_pack_start(GTK_BOX(vbox), fixed, FALSE, FALSE, 0);

	label = gtk_label_new("    ");
	gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

	/* OK, Cancel button */
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_set_spacing(GTK_BOX(hbox), 25);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);

	for (int i = 0; i < 2; i++) {
		label = gtk_label_new("    ");
		gtk_widget_show(label);
		gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	}

	OK_button = gtk_button_new_with_label("OK");
	gtk_widget_set_size_request(OK_button, 60, 30);
	gtk_widget_show(OK_button);
	gtk_box_pack_start(GTK_BOX(hbox), OK_button, FALSE, FALSE, 0);
	Cancel_button = gtk_button_new_with_label("Cancel");
	gtk_widget_set_size_request(Cancel_button, 60, 30);
	gtk_box_pack_start(GTK_BOX(hbox), Cancel_button, FALSE, FALSE, 0);
	gtk_widget_show(Cancel_button);
	gtk_widget_show(hbox);

	g_signal_connect(GTK_OBJECT(about_button), "clicked",
		GTK_SIGNAL_FUNC(about_button_action), NULL);
	g_signal_connect(GTK_OBJECT(OK_button), "clicked",
		GTK_SIGNAL_FUNC(OK_action), NULL);
	g_signal_connect(GTK_OBJECT(Cancel_button), "clicked",
		GTK_SIGNAL_FUNC(Cancel_action), NULL);

	gtk_container_add(GTK_CONTAINER(window_help), vbox);
	help_window_text_change();
	gtk_widget_show(vbox);

	return window_help;
}

#ifdef SIMULATION
int main( int argc, char *argv[])
{
    gtk_init(&argc, &argv);
    window_help = create_setting_window();
    gtk_widget_show_all(window_help);
    gtk_main();

    return 0;
}
#endif
