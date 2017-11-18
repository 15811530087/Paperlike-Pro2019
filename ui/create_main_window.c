#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "params.h"
#include "ddcci.h"
#include "thread.h"
#include "manage_window_show_hide.h"

GtkWidget *window	= NULL;
GtkWidget *button_A2 	= NULL;
GtkWidget *button_A5	= NULL;
GtkWidget *button_A16	= NULL;
GtkWidget *button_Set	= NULL;
GtkWidget *button_Hide	= NULL;
GtkWidget *button_Help	= NULL;
GtkWidget *button_Exit	= NULL;
GtkWidget *label_info	= NULL;
GtkWidget *radio_high_res = NULL;
GtkWidget *radio_low_res  = NULL;

GtkWidget *vbox		= NULL;
char ui_changed_flag	= 0;

extern void restore_default_screen_resolution();
extern void set_default_screen_resolution();
//extern int resolution_change(char *resolution);

static void ui_destroy_signal(void)
{
	DSPRINT("Click destroy signal.\n");
	gtk_main_quit();
}

void main_window_text_change();

void cancel_all_button_sensitive()
{
	gtk_widget_set_sensitive(button_A5, TRUE);
	gtk_widget_set_sensitive(button_A2, TRUE);
	gtk_widget_set_sensitive(button_A16, TRUE);
}

void main_window_init()
{
	main_window_text_change();
	switch (ResCommMode) {
	case DS_RES_HIGH:
		gtk_button_set_label(GTK_BUTTON(button_A5), "Floyd");
		gtk_button_set_label(GTK_BUTTON(button_A16), "A16");
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_high_res), FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_high_res), TRUE);
		break;
	case DS_RES_LOW:
		gtk_button_set_label(GTK_BUTTON(button_A5), "A5");
		gtk_button_set_label(GTK_BUTTON(button_A16), "A61");
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_low_res), TRUE);
		break;
	}
	cancel_all_button_sensitive();
	switch (show_mode) {
	case DS_MODE_FLOYD:
	case DS_MODE_LOW_A5:
		gtk_widget_set_sensitive(button_A5, FALSE);
		break;
	case DS_MODE_A2:
	case DS_MODE_LOW_A2:
		gtk_widget_set_sensitive(button_A2, FALSE);
		break;
	case DS_MODE_A16:
	case DS_MODE_LOW_A61:
		gtk_widget_set_sensitive(button_A16, FALSE);
	default:
		break;
	}
}

static int paperlike_refresh_ui(GtkWidget *textview)
{
	usleep(500*1000);

	if (ui_changed_flag == 0)
		return 1;

	ui_changed_flag = 0;
	main_window_init();
	return 1;
}


void main_window_text_change()
{
	if (language == LANG_ZH) {
		gtk_button_set_label(GTK_BUTTON(button_Set), MAIN_UI_SET_CH);
		gtk_button_set_label(GTK_BUTTON(radio_high_res), MAIN_UI_RES_HIGH_CH);
		gtk_button_set_label(GTK_BUTTON(radio_low_res), MAIN_UI_RES_LOW_CH);
	} else {
		gtk_button_set_label(GTK_BUTTON(button_Set), MAIN_UI_SET_EN);
		gtk_button_set_label(GTK_BUTTON(radio_high_res), MAIN_UI_RES_HIGH_EN);
		gtk_button_set_label(GTK_BUTTON(radio_low_res), MAIN_UI_RES_LOW_EN);
	}
}

gboolean deal_key_press(GtkWidget *widget, GdkEventKey  *event, gpointer data)
{
    switch(event->keyval){   // 键盘键值类型
        case GDK_Up:
            DSPRINT("Up\n");
            break;
        case GDK_Left:
            DSPRINT("Left\n");
            break;
        case GDK_Right:
            DSPRINT("Right\n");
            break;
        case GDK_Down:
            DSPRINT("Down\n");
            break;
    }
#if 0
case GDK_m:
      if (event->state & GDK_SHIFT_MASK)
      {
        DSPRINT("key pressed: %s\n", "shift + m");
      }
      else if (event->state & GDK_CONTROL_MASK)
      {
        DSPRINT("key pressed: %s\n", "ctrl + m");
      }
      else
      {
        DSPRINT("key pressed: %s\n", "m");
      }
      break;
#endif
    int key = event->keyval; // 获取键盘键值类型
    DSPRINT("keyval = %d\n", key);

    return TRUE;
}

static void radio_action(GtkWidget *widget, gpointer data)
{
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)) == false)
		return;
	DSPRINT("high(%p), low(%p), widget = %p, data = %p.\n",
		radio_high_res, radio_low_res, widget, data);
	if (MonitorOnline == false) {
		/* Create dialog for action */
		GtkWidget *dialog = NULL;
		dialog = gtk_message_dialog_new (GTK_WINDOW (window), GTK_DIALOG_MODAL,
			GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
			"Dasung monitor not detected!");
		//gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		return;
	}
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_high_res)) &&
		(widget == radio_high_res))
	{
		gtk_button_set_label(GTK_BUTTON(button_A5), "Floyd");
		gtk_button_set_label(GTK_BUTTON(button_A16), "A16");
		restore_default_screen_resolution();
		//resolution_change("1600x1200");
		ResCommMode = DS_RES_HIGH;
		resolution_change_flag = 1; 
	}

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_low_res)) &&
		(widget == radio_low_res))
	{
		gtk_button_set_label(GTK_BUTTON(button_A5), "A5");
		gtk_button_set_label(GTK_BUTTON(button_A16), "A61");
		//resolution_change("800x600");
		ResCommMode = DS_RES_LOW;
		set_default_screen_resolution();
		DSPRINT("-------------- ++++  send monitor resolution.\n");
		resolution_change_flag = 1; 
	}
	switch (ResCommMode) {
	case DS_RES_HIGH:
		switch (show_mode) {
		case DS_MODE_LOW_A2:
			show_mode = DS_MODE_A2;
			set_threshold_value = DS_MODE_DEF_THRESHOLD_A2;
			threshold = DS_MODE_DEF_THRESHOLD_A2;
			//set_threshold_flag = 1;
			break;
		case DS_MODE_LOW_A5:
			show_mode = DS_MODE_FLOYD;
			set_threshold_value = DS_MODE_DEF_THRESHOLD_FLOYD;
			threshold = DS_MODE_DEF_THRESHOLD_FLOYD;
			//set_threshold_flag = 1;
			break;
		case DS_MODE_LOW_A61:
			show_mode = DS_MODE_A16;
			break;
		}
		break;
	case DS_RES_LOW:
		switch (show_mode) {
		case DS_MODE_A2:
			show_mode = DS_MODE_LOW_A2;
			set_threshold_value = DS_MODE_DEF_THRESHOLD_A2;
			threshold = DS_MODE_DEF_THRESHOLD_A2;
			//set_threshold_flag = 1;
			break;
		case DS_MODE_FLOYD:
			show_mode = DS_MODE_LOW_A5;
			set_threshold_value = DS_MODE_DEF_THRESHOLD_A5;
			threshold = DS_MODE_DEF_THRESHOLD_A5;
			//set_threshold_flag = 1;
			break;
		case DS_MODE_A16:
			show_mode = DS_MODE_LOW_A61;
		break;
		}
		break;
	}

	saveParameters();
}

static void A2_action(GtkWidget *wid, GtkWidget *win)
{
	if (MonitorOnline == false) {
		/* Create dialog for action */
		GtkWidget *dialog = NULL;
		dialog = gtk_message_dialog_new (GTK_WINDOW (win), GTK_DIALOG_MODAL,
			GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
			"Dasung monitor not detected!");
		gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		return;
	}

	if (show_mode == DS_MODE_A16 || show_mode == DS_MODE_LOW_A61)
		usleep(500);

	switch (ResCommMode) {
	case DS_RES_HIGH:
		show_mode = DS_MODE_A2;
		break;
	case DS_RES_LOW:
		show_mode = DS_MODE_LOW_A2;
		break;
	default:
		DSPRINT("unknown ResCommMode(%d).\n", ResCommMode);
		return;
	}

	work_mode = DS_DISPLAY_NORMAL;
	cancel_all_button_sensitive();
	gtk_widget_set_sensitive(button_A2, FALSE);
	
	set_mode_flag = 1;
	saveParameters();
	sleep(1);
}

static void A5_action(GtkWidget *wid, GtkWidget *win)
{
	//unsigned char replay_data[11];
	//unsigned char replay_size = 8;

	if (MonitorOnline == false) {
		/* Create dialog for action */
		GtkWidget *dialog = NULL;
		dialog = gtk_message_dialog_new (GTK_WINDOW (win), GTK_DIALOG_MODAL,
			GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
			"Dasung monitor not detected!");
		gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		return;
	}

	if (show_mode == DS_MODE_A16 || show_mode == DS_MODE_LOW_A61)
		usleep(500);

	switch (ResCommMode) {
	case DS_RES_HIGH:
		show_mode = DS_MODE_FLOYD;
		break;
	case DS_RES_LOW:
		show_mode = DS_MODE_LOW_A5;
		break;
	default:
		DSPRINT("unknown ResCommMode(%d).\n", ResCommMode);
		return;
	}
	work_mode = DS_DISPLAY_NORMAL;

	cancel_all_button_sensitive();
	gtk_widget_set_sensitive(button_A5, FALSE);
	set_mode_flag = 1;
	saveParameters();
	sleep(1);
}

static void A16_action(GtkWidget *wid, GtkWidget *win)
{
	if (MonitorOnline == false) {
		/* Create dialog for action */
		GtkWidget *dialog = NULL;
		dialog = gtk_message_dialog_new (GTK_WINDOW (win), GTK_DIALOG_MODAL,
			GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
			"Dasung monitor not detected!");
		gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		return;
	}

	switch (ResCommMode) {
	case DS_RES_HIGH:
		show_mode = DS_MODE_A16;
		break;
	case DS_RES_LOW:
		show_mode = DS_MODE_LOW_A61;
		break;
	default:
		DSPRINT("unknown ResCommMode(%d).\n", ResCommMode);
		return;
	}

	work_mode = DS_DISPLAY_NORMAL;

	cancel_all_button_sensitive();
	gtk_widget_set_sensitive(button_A16, FALSE);

	set_mode_flag = 1;
	saveParameters();
	sleep(1);
}

static void Set_action(GtkWidget *wid, GtkWidget *win)
{
	/* Create dialog for action */
	setting_window_create_event();
}

static void Help_action(GtkWidget *wid, GtkWidget *win)
{
	help_window_create_event();
}

static void Hide_action(GtkWidget *wid, GtkWidget *win)
{
	/* Create dialog for action */
	GtkWidget *dialog = NULL;
	dialog = gtk_message_dialog_new (GTK_WINDOW (win), GTK_DIALOG_MODAL,
		GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, "Create Hide button!");
	gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
}

static void Exit_action(GtkWidget *wid, GtkWidget *win)
{
	/* Create dialog for action */
	GtkWidget *dialog = NULL;
	dialog = gtk_message_dialog_new (GTK_WINDOW (win), GTK_DIALOG_MODAL,
		GTK_MESSAGE_INFO, GTK_BUTTONS_YES_NO,
		//GTK_MESSAGE_INFO, GTK_BUTTONS_OK | GTK_BUTTONS_CLOSE,
		"Are you sure to quit PaperLikeHD app?");
	gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_YES)
		gtk_main_quit();

	gtk_widget_destroy (dialog);
}


GtkWidget *create_main_window()
{
	GSList *group;
	/* Create main window */
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	/* no resizable */
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	/* Set window border width */
	gtk_container_set_border_width(GTK_CONTAINER(window), 8);
	/* Set window title */
	gtk_window_set_title(GTK_WINDOW(window), WND_TITLE);
	/* Set window position */
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	/* Realize window */
	gtk_widget_realize(window);
	/* Connect destroy signal */
	g_signal_connect(window, "destroy", ui_destroy_signal, NULL);

	/* Create a vertical box with buttons */
	vbox = gtk_vbox_new(TRUE, 5);
	/* Add vbox into window */
	gtk_container_add(GTK_CONTAINER(window), vbox);

	//label_info = gtk_label_new("");
	//gtk_box_pack_start(GTK_BOX(vbox), label_info, TRUE, TRUE,
	//	WINDOW_VBOX_PADDING);

	//vbox = gtk_vbox_new(TRUE, 12);
	/* Add vbox into window */
	//gtk_container_add(GTK_CONTAINER(window), vbox);

	/* Create high res radio */
	radio_high_res = gtk_radio_button_new_with_label(NULL, "可变分辨率模式");
	//gtk_widget_set_size_request(radio_high_res, 80, 20);
	gtk_box_pack_start(GTK_BOX(vbox), radio_high_res, TRUE, TRUE, 0);
	gtk_widget_show(radio_high_res);
	g_signal_connect(GTK_OBJECT(radio_high_res), "clicked",
		GTK_SIGNAL_FUNC(radio_action), NULL);

	/* Create low res radio */
	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio_high_res));
	radio_low_res = gtk_radio_button_new_with_label(group, "固定1100X825模式");
	//gtk_widget_set_size_request(radio_low_res, 80, 20);
	gtk_box_pack_start(GTK_BOX(vbox), radio_low_res, TRUE, TRUE, 0);
	gtk_widget_show(radio_low_res);
	//g_signal_connect(GTK_OBJECT(radio_low_res), "clicked",
	//	GTK_SIGNAL_FUNC(radio_action), NULL);

	/* Create A5 button */
	button_A5 = gtk_button_new_from_stock(MAIN_UI_A5_EN);
	g_signal_connect(G_OBJECT(button_A5), "clicked", G_CALLBACK(A5_action),
		(gpointer)window);
	gtk_box_pack_start(GTK_BOX(vbox), button_A5, TRUE, TRUE,
		WINDOW_VBOX_PADDING);

	/* Create A2 button */
	button_A2 = gtk_button_new_from_stock(MAIN_UI_A2_EN);
	g_signal_connect(G_OBJECT(button_A2), "clicked", G_CALLBACK(A2_action),
		(gpointer)window);
	gtk_box_pack_start(GTK_BOX(vbox), button_A2, TRUE, TRUE,
		WINDOW_VBOX_PADDING);

#if 0
	/* Create A5 button */
	button_A5 = gtk_button_new_from_stock(MAIN_UI_A5_EN);
	g_signal_connect(G_OBJECT(button_A5), "clicked", G_CALLBACK(A5_action),
		(gpointer)window);
	gtk_box_pack_start(GTK_BOX(vbox), button_A5, TRUE, TRUE,
		WINDOW_VBOX_PADDING);
#endif
	/* Create A16 button */
	button_A16 = gtk_button_new_from_stock(MAIN_UI_A16_EN);
	g_signal_connect(G_OBJECT(button_A16), "clicked", G_CALLBACK(A16_action),
		(gpointer)window);
	gtk_box_pack_start(GTK_BOX(vbox), button_A16, TRUE, TRUE,
		WINDOW_VBOX_PADDING);

	/* Create HIDE button */
	#if 0
	button_Hide = gtk_button_new_from_stock(MAIN_UI_HIDE_EN);
	g_signal_connect(G_OBJECT(button_Hide), "clicked", G_CALLBACK(Hide_action),
		(gpointer)window);
	gtk_box_pack_start(GTK_BOX(vbox), button_Hide, TRUE, TRUE,
		WINDOW_VBOX_PADDING);
	#endif

	/* Create SETTING button */
	button_Set = gtk_button_new_from_stock(MAIN_UI_SET_EN);
	g_signal_connect(G_OBJECT(button_Set), "clicked", G_CALLBACK(Set_action),
		(gpointer)window);
	gtk_box_pack_start(GTK_BOX(vbox), button_Set, TRUE, TRUE,
		WINDOW_VBOX_PADDING);

	/* Create Help button */
	button_Help = gtk_button_new_from_stock(MAIN_UI_HELP_EN);
	g_signal_connect(G_OBJECT(button_Help), "clicked", G_CALLBACK(Help_action),
		(gpointer)window);
	gtk_box_pack_start(GTK_BOX(vbox), button_Help, true, true,
		WINDOW_VBOX_PADDING);

	/* Create EXIT button */
	button_Exit = gtk_button_new_from_stock(MAIN_UI_EXIT_EN);
	g_signal_connect(G_OBJECT(button_Exit), "clicked", G_CALLBACK(Exit_action),
		(gpointer)window);
	gtk_box_pack_start(GTK_BOX(vbox), button_Exit, TRUE, TRUE,
		WINDOW_VBOX_PADDING);

	/* Create LABEL */
	label_info = gtk_label_new("PaperLikeHD v1.0.6");
	gtk_box_pack_start(GTK_BOX(vbox), label_info, TRUE, TRUE,
		WINDOW_VBOX_PADDING);

	main_window_text_change();
	//main_window_init();

	g_signal_connect(GTK_OBJECT(radio_low_res), "clicked",
		GTK_SIGNAL_FUNC(radio_action), NULL);

	g_idle_add((GSourceFunc)paperlike_refresh_ui, NULL);
	//g_signal_connect((gtk_widget_get_root_window(window)), "key-press-event",
        //            G_CALLBACK(deal_key_press), NULL);
	/* Show all widget on window */
	gtk_widget_show_all(window);

	return window;
}
