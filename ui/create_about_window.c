#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "params.h"
#include "ddcci.h"
#include "thread.h"
#include "manage_window_show_hide.h"

GtkWidget *window_about	= NULL;
GtkWidget *OK_button	= NULL;

static void Cancel_action(GtkWidget *widget, gpointer data)
{
	about_window_delete_event();
}

static void OK_action(GtkWidget *widget, gpointer data)
{
	about_window_delete_event();
}


GtkWidget *create_about_window()
{
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *label_info;
	GtkWidget *fixed;

	window_about = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_resizable(GTK_WINDOW(window_about), FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(window_about), 30);
	gtk_window_set_position(GTK_WINDOW(window_about), GTK_WIN_POS_CENTER);

	vbox = gtk_vbox_new(FALSE, 15);

	fixed = gtk_fixed_new();
	label_info = gtk_label_new("PaperLikeHD");
	gtk_widget_set_size_request(label_info, 110, 30);
	gtk_fixed_put(GTK_FIXED(fixed), label_info, 0, 0);
	gtk_box_pack_start(GTK_BOX(vbox), fixed, FALSE, FALSE, 0);

	fixed = gtk_fixed_new();
	label_info = gtk_label_new("     v1.0.8");
	gtk_widget_set_size_request(label_info, 80, 30);
	gtk_fixed_put(GTK_FIXED(fixed), label_info, 0, 0);
	gtk_box_pack_start(GTK_BOX(vbox), fixed, TRUE, TRUE,
		WINDOW_VBOX_PADDING);

	fixed = gtk_fixed_new();
	label_info = gtk_label_new("Copyright (C) 2017, 2018, DASUNG Ltd.\nAll Rights Reserved.");
	gtk_widget_set_size_request(label_info, 260, 50);
	gtk_fixed_put(GTK_FIXED(fixed), label_info, 0, 0);
	gtk_box_pack_start(GTK_BOX(vbox), fixed, TRUE, TRUE,
		WINDOW_VBOX_PADDING);

	fixed = gtk_fixed_new();
	label_info = gtk_label_new("Contact:\nhttp://www.dasung.com/");
	gtk_widget_set_size_request(label_info, 170, 40);
	gtk_fixed_put(GTK_FIXED(fixed), label_info, 0, 0);
	gtk_box_pack_start(GTK_BOX(vbox), fixed, TRUE, TRUE,
		WINDOW_VBOX_PADDING);

	/* OK, Cancel button */
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_set_spacing(GTK_BOX(hbox), 25);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);

	for (int i = 0; i < 4; i++) {
		label = gtk_label_new("    ");
		gtk_widget_show(label);
		gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	}

	OK_button = gtk_button_new_with_label("OK");
	gtk_widget_show(OK_button);
	gtk_box_pack_start(GTK_BOX(hbox), OK_button, FALSE, FALSE, 0);

	g_signal_connect(GTK_OBJECT(OK_button), "clicked",
		GTK_SIGNAL_FUNC(OK_action), NULL);

	gtk_container_add(GTK_CONTAINER(window_about), vbox);
	gtk_widget_show(vbox);

	return window_about;
}
