#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include "thread.h"
#include "manage_window_show_hide.h"

GtkWidget *window_setting;
GtkWidget *static_current_work_mode;
//GtkWidget *radio_threshold;
GtkWidget *radio_low;
GtkWidget *radio_medium;
GtkWidget *radio_deep;
GtkWidget *radio_selfdef;
GtkWidget *slider_threshold;
GtkWidget *edit_threshold;
GtkWidget *checkbox_auto_refresh;
GtkWidget *edit_auto_refresh_time;
GtkWidget *checkbox_soft_hand_refresh;
GtkWidget *checkbox_hard_hand_refresh;
GtkWidget *OK_button;
GtkWidget *Cancel_button;
GtkWidget *normal_display_label;
GtkWidget *current_mode_label;
GtkWidget *constrait_label;
GtkWidget *auto_refresh_interval_label;

void setting_window_text_change()
{
	if (language == LANG_ZH) {
		gtk_label_set_text(GTK_LABEL(normal_display_label), SET_NOR_DISP_CH);
		gtk_label_set_text(GTK_LABEL(current_mode_label), SET_CUR_MODE_CH);
		gtk_label_set_text(GTK_LABEL(constrait_label), SET_CONSTRAIT_CH);
		gtk_button_set_label(GTK_BUTTON(radio_low), SET_LOW_CH);
		gtk_button_set_label(GTK_BUTTON(radio_medium), SET_MEDIUM_CH);
		gtk_button_set_label(GTK_BUTTON(radio_deep), SET_DARK_CH);
		gtk_button_set_label(GTK_BUTTON(radio_selfdef), SET_SELFDEF_CH);
		gtk_button_set_label(GTK_BUTTON(checkbox_auto_refresh), SET_AUTO_REFRESH_CH);
		gtk_label_set_text(GTK_LABEL(auto_refresh_interval_label), SET_AUTO_REFRESH_CH);
		gtk_button_set_label(GTK_BUTTON(checkbox_soft_hand_refresh), SET_SOFT_HAND_CH);
		gtk_button_set_label(GTK_BUTTON(checkbox_hard_hand_refresh), SET_HARD_HAND_CH);
	} else {
		gtk_label_set_text(GTK_LABEL(normal_display_label), SET_NOR_DISP_EN);
		gtk_label_set_text(GTK_LABEL(current_mode_label), SET_CUR_MODE_EN);
		gtk_label_set_text(GTK_LABEL(constrait_label), SET_CONSTRAIT_EN);
		gtk_button_set_label(GTK_BUTTON(radio_low), SET_LOW_EN);
		gtk_button_set_label(GTK_BUTTON(radio_medium), SET_MEDIUM_EN);
		gtk_button_set_label(GTK_BUTTON(radio_deep), SET_DARK_EN);
		gtk_button_set_label(GTK_BUTTON(radio_selfdef), SET_SELFDEF_EN);
		gtk_button_set_label(GTK_BUTTON(checkbox_auto_refresh), SET_AUTO_REFRESH_EN);
		gtk_label_set_text(GTK_LABEL(auto_refresh_interval_label), SET_AUTO_REFRESH_EN);
		gtk_button_set_label(GTK_BUTTON(checkbox_soft_hand_refresh), SET_SOFT_HAND_EN);
		gtk_button_set_label(GTK_BUTTON(checkbox_hard_hand_refresh), SET_HARD_HAND_EN);
	}
}

static void radio_action(GtkWidget *widget, gpointer data)
{
	char buf[5];
	int n = gtk_range_get_value(GTK_RANGE(slider_threshold));
	//gtk_main_quit();
	//DSPRINT("received radio click action.\n");
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_low)) &&
			(widget == radio_low)) {
		n = DS_MODE_THRESHOLD_LOW;
		//n = 130;
		DSPRINT("low\n");
		set_threshold_value = n;
		set_threshold_flag = 1;
	}
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_medium)) &&
			(widget == radio_medium)) {
		n = DS_MODE_THRESHOLD_MEDIUM;
		//n = 190;
		DSPRINT("medium\n");
		set_threshold_value = n;
		set_threshold_flag = 1;
	}
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_deep)) &&
			(widget == radio_deep)) {
		n = DS_MODE_THRESHOLD_HIGH;
		//n = 230;
		DSPRINT("deep\n");
		set_threshold_value = n;
		set_threshold_flag = 1;
	}
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_selfdef)) &&
			(widget == radio_selfdef)) {
		DSPRINT("selfdef radio\n");
	}
	snprintf(buf, 5, "%d", n);
	gtk_entry_set_text(GTK_ENTRY(edit_threshold), buf);
	gtk_range_set_value(GTK_RANGE(slider_threshold), n);
}

void slider_threshold_action(GtkWidget *widget, gpointer data)
{
	int n = gtk_range_get_value(GTK_RANGE(slider_threshold));
	char buf[5];

	snprintf(buf, 5, "%d", n);
	gtk_entry_set_text(GTK_ENTRY(edit_threshold), buf);
	DSPRINT("%s : %d.\n", __func__, n);
	switch (n) {
	case DS_MODE_THRESHOLD_LOW:
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_low), TRUE);
		break;
	case DS_MODE_THRESHOLD_MEDIUM:
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_medium), TRUE);
		break;
	case DS_MODE_THRESHOLD_HIGH:
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_deep), TRUE);
		break;
	default:
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_selfdef), TRUE);
		break;
	}

	set_threshold_value = n;
	set_threshold_flag = 1;

	//gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_selfdef), TRUE);
}

void edit_threshold_action(GtkWidget *widget, gpointer data)
{
	int edit_threshold_value = strtol(gtk_entry_get_text(GTK_ENTRY(
					edit_threshold)), NULL, 10);

	if (edit_threshold_value > 0 && edit_threshold_value < 256) {
		gtk_range_set_value(GTK_RANGE(slider_threshold),
			edit_threshold_value);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_selfdef), TRUE);
		set_threshold_value = edit_threshold_value;
		set_threshold_flag = 1;
	}
	DSPRINT("%s.\n", __func__);
}

void checkbox_auto_refresh_action(GtkWidget *widget, gpointer data)
{
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
			checkbox_auto_refresh)))
		gtk_entry_set_editable(GTK_ENTRY(edit_auto_refresh_time), TRUE);
	else
		gtk_entry_set_editable(GTK_ENTRY(edit_auto_refresh_time), FALSE);

	DSPRINT("%s.\n", __func__);
}

void setting_window_init(void)
{
	char buf[5];
#if 1
	if (soft_hand_refresh_enable)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
			checkbox_soft_hand_refresh), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
			checkbox_soft_hand_refresh), FALSE);

	if (hard_hand_refresh_enable)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
			checkbox_hard_hand_refresh), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
			checkbox_hard_hand_refresh), FALSE);

	if (auto_clear_blur_enable) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
			checkbox_auto_refresh), TRUE);
		gtk_entry_set_editable(GTK_ENTRY(edit_auto_refresh_time), TRUE);
	} else {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
			checkbox_auto_refresh), FALSE);
		gtk_entry_set_editable(GTK_ENTRY(edit_auto_refresh_time), FALSE);
	}

	snprintf(buf, 5, "%d", auto_clear_blur_time);
	gtk_entry_set_text(GTK_ENTRY(edit_auto_refresh_time), buf);


	switch (threshold) {
	case DS_MODE_THRESHOLD_LOW:
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_low), TRUE);
		break;
	case DS_MODE_THRESHOLD_MEDIUM:
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_medium), TRUE);
		break;
	case DS_MODE_THRESHOLD_HIGH:
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_deep), TRUE);
		break;
	default:
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_selfdef), TRUE);
		break;
	}

	snprintf(buf, 5, "%d", threshold);
	gtk_entry_set_text(GTK_ENTRY(edit_threshold), buf);
	gtk_range_set_value(GTK_RANGE(slider_threshold), threshold);
	switch (show_mode) {
	case DS_MODE_A2:
		gtk_label_set_text(GTK_LABEL(static_current_work_mode), "         A2");
		break;
	case DS_MODE_FLOYD:
		gtk_label_set_text(GTK_LABEL(static_current_work_mode), "         FLOYD");
		break;
	case DS_MODE_A16:
		gtk_label_set_text(GTK_LABEL(static_current_work_mode), "         A16");
		break;
	case DS_MODE_LOW_A2:
		gtk_label_set_text(GTK_LABEL(static_current_work_mode), "         A2");
		break;
	case DS_MODE_LOW_A5:
		gtk_label_set_text(GTK_LABEL(static_current_work_mode), "         A5");
		break;
	case DS_MODE_LOW_A61:
		gtk_label_set_text(GTK_LABEL(static_current_work_mode), "         A61");
		break;
	}

	switch (show_mode) {
	case DS_MODE_LOW_A2:
	case DS_MODE_LOW_A5:
	case DS_MODE_A2:
	case DS_MODE_FLOYD:
		gtk_widget_set_sensitive(radio_low, TRUE);
		gtk_widget_set_sensitive(radio_medium, TRUE);
		gtk_widget_set_sensitive(radio_deep, TRUE);
		gtk_widget_set_sensitive(radio_selfdef, TRUE);
		gtk_widget_set_sensitive(slider_threshold, TRUE);
		gtk_widget_set_sensitive(edit_threshold, TRUE);
		break;
	default:
		gtk_widget_set_sensitive(radio_low, FALSE);
		gtk_widget_set_sensitive(radio_medium, FALSE);
		gtk_widget_set_sensitive(radio_deep, FALSE);
		gtk_widget_set_sensitive(radio_selfdef, FALSE);
		gtk_widget_set_sensitive(slider_threshold, FALSE);
		gtk_widget_set_sensitive(edit_threshold, FALSE);
		break;
	}
#endif
}

static void OK_action(GtkWidget *widget, gpointer data)
{
	DSPRINT("%s.\n", __func__);
	//gtk_toggle_button_set_active(checkbox_soft_hand_refresh, TRUE);
	//if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_low)) &&
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbox_soft_hand_refresh))) {
		DSPRINT("soft_hand_refresh checked.\n");
		soft_hand_refresh_enable = 1;
	} else {
		soft_hand_refresh_enable = 0;
		DSPRINT("soft_hand_refresh unchecked.\n");
	}

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbox_hard_hand_refresh))) {
		hard_hand_refresh_enable = 1;
		DSPRINT("hard_hand_refresh checked.\n");
	} else {
		hard_hand_refresh_enable = 0;
		DSPRINT("hard_hand_refresh unchecked.\n");
	}

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbox_auto_refresh))) {
		auto_clear_blur_enable = 1;
		auto_clear_blur_time = strtol(gtk_entry_get_text(GTK_ENTRY(
				edit_auto_refresh_time)), NULL, 10);
		DSPRINT("auto_refresh checked, time is %s.\n",
			gtk_entry_get_text(GTK_ENTRY(edit_auto_refresh_time)));
	} else {
		DSPRINT("auto_refresh unchecked.\n");
		auto_clear_blur_enable = 0;
	}

	threshold = gtk_range_get_value(GTK_RANGE(slider_threshold));

	saveParameters();
	//setting_window_init();
	setting_window_delete_event();
}

static void Cancel_action(GtkWidget *widget, gpointer data)
{
	if (threshold != gtk_range_get_value(GTK_RANGE(slider_threshold))) {
		set_threshold_value = threshold;
		set_threshold_flag = 1;
	}
	setting_window_delete_event();
	setting_window_init();
	DSPRINT("%s.\n", __func__);
}

GtkWidget *create_setting_window()
{
	GtkWidget *vbox;
	GtkWidget *vbox_sub;
	GtkWidget *button;
	GtkWidget *separator;
	GtkWidget *hbox;
	GtkWidget *label;
	GSList *group;
	window_setting = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_resizable(GTK_WINDOW(window_setting), FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(window_setting),30);
	gtk_window_set_position (GTK_WINDOW(window_setting), GTK_WIN_POS_CENTER);

	//Add the global vbox
	vbox = gtk_vbox_new(FALSE, 10);

	/* Add the seting description hbox */
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	//label = gtk_label_new("正常显示设置:");
	label = gtk_label_new(SET_NOR_DISP_CH);
	gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
	gtk_label_set_justify(GTK_LABEL(label),GTK_JUSTIFY_LEFT);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	normal_display_label = label;
	gtk_widget_show(label);
	gtk_widget_show(hbox);

	/* Add seperator */
	separator = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, TRUE, 0);
	gtk_widget_show(separator);

	/* Add the current show mode description hbox */
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	label = gtk_label_new(SET_CUR_MODE_CH);
	current_mode_label = label;
	gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
	gtk_label_set_justify(GTK_LABEL(label),GTK_JUSTIFY_LEFT);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);
	gtk_widget_show(hbox);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	static_current_work_mode = gtk_label_new("         A2");
	gtk_box_pack_start(GTK_BOX(hbox), static_current_work_mode, FALSE, FALSE, 0);
	gtk_widget_show(static_current_work_mode);
	gtk_widget_show(hbox);

	/* Add seperator */
	separator = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, TRUE, 0);
	gtk_widget_show(separator);

	/* Add threshold radio hbox */
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	//label = gtk_label_new("当前显示阈值调整：");
	label = gtk_label_new(SET_CONSTRAIT_CH);
	constrait_label = label;
	gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
	gtk_label_set_justify(GTK_LABEL(label),GTK_JUSTIFY_LEFT);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);
	gtk_widget_show(hbox);

	hbox = gtk_hbox_new(FALSE, 10);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	radio_low = gtk_radio_button_new_with_label(NULL, SET_LOW_CH);
	gtk_widget_set_size_request(radio_low , 80, 20);
	gtk_box_pack_start(GTK_BOX(hbox), radio_low, TRUE, TRUE, 0);
	gtk_widget_show(radio_low);
  
	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio_low));
	radio_medium = gtk_radio_button_new_with_label(group, SET_MEDIUM_CH);
	gtk_widget_set_size_request(radio_medium, 80, 20);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (radio_medium), TRUE);
	gtk_box_pack_start(GTK_BOX (hbox), radio_medium, TRUE, TRUE, 0);
	gtk_widget_show(radio_medium);

	radio_deep = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (radio_medium), SET_DARK_CH);
	gtk_widget_set_size_request(radio_deep, 80, 20);
	gtk_box_pack_start(GTK_BOX (hbox), radio_deep, TRUE, TRUE, 0);
	gtk_widget_show(radio_deep);
   
	radio_selfdef = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (radio_deep), SET_SELFDEF_CH);
	gtk_widget_set_size_request(radio_selfdef, 80, 20);
	gtk_box_pack_start(GTK_BOX (hbox), radio_selfdef, TRUE, TRUE, 0);
	gtk_widget_show (radio_selfdef);
	gtk_widget_show(hbox);

	//Add slider button
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	slider_threshold = gtk_hscale_new_with_range(1, 255, 1);
	gtk_range_set_update_policy(GTK_RANGE(slider_threshold), GTK_UPDATE_DELAYED);
	gtk_box_pack_start(GTK_BOX (hbox), slider_threshold, TRUE, TRUE, 0);
	gtk_widget_show(slider_threshold);
	GtkWidget *align = gtk_alignment_new(1, 1, 0, 0);
	//Add edit control
	edit_threshold = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(edit_threshold), "200");
	gtk_entry_set_max_length(GTK_ENTRY(edit_threshold), 5);
	gtk_widget_set_size_request(edit_threshold, 80, 30);
	gtk_box_pack_start(GTK_BOX(hbox), align, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(align),edit_threshold);
	gtk_widget_show(align);
	gtk_widget_show(hbox);
	gtk_widget_show(edit_threshold);

	/* Add seperator */
	separator = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX (vbox), separator, FALSE, TRUE, 0);
	gtk_widget_show(separator);

	/* Add checkbox control */
	hbox = gtk_hbox_new(FALSE, 0);
	// Auto soft refresh checkbox
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	checkbox_auto_refresh = gtk_check_button_new_with_label("定时自动刷新");
	gtk_box_pack_start(GTK_BOX(hbox), checkbox_auto_refresh, FALSE, FALSE, 0);
	// Add Label
	label = gtk_label_new("          自动刷新间隔：");
	auto_refresh_interval_label = label;
	gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
	gtk_label_set_justify(GTK_LABEL(label),GTK_JUSTIFY_RIGHT);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);
	// Add entry control
	edit_auto_refresh_time = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(edit_auto_refresh_time), "15");
	gtk_entry_set_max_length(GTK_ENTRY(edit_auto_refresh_time), 3);
	gtk_widget_set_size_request(edit_auto_refresh_time, 50, 30);
	gtk_box_pack_start(GTK_BOX(hbox), edit_auto_refresh_time, FALSE, FALSE, 0);
	gtk_widget_show(edit_auto_refresh_time);
	// Add Label
	label = gtk_label_new("s  (15s ~ 600s)");
	gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
	gtk_label_set_justify(GTK_LABEL(label),GTK_JUSTIFY_LEFT);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);
	gtk_widget_show(hbox);

	// Soft refresh checkbox
	checkbox_soft_hand_refresh = gtk_check_button_new_with_label(SET_SOFT_HAND_CH);
	gtk_box_pack_start(GTK_BOX(vbox), checkbox_soft_hand_refresh, FALSE, TRUE, 0);
	gtk_widget_show(checkbox_soft_hand_refresh);
	// Hard refresh checkbox
	checkbox_hard_hand_refresh = gtk_check_button_new_with_label(SET_HARD_HAND_CH);
	gtk_box_pack_start(GTK_BOX(vbox), checkbox_hard_hand_refresh, FALSE, TRUE, 0);
	gtk_widget_show(checkbox_hard_hand_refresh);

	/* Add seperator */
	separator = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX (vbox), separator, FALSE, TRUE, 0);
	gtk_widget_show(separator);


	/* Add OK Cancel button */
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
	Cancel_button = gtk_button_new_with_label("Cancel");
	gtk_box_pack_start(GTK_BOX(hbox), Cancel_button, FALSE, FALSE, 0);
	gtk_widget_show(Cancel_button);
	gtk_widget_show(hbox);



	g_signal_connect(GTK_OBJECT(radio_low),"clicked",
		GTK_SIGNAL_FUNC(radio_action),NULL);
	g_signal_connect(GTK_OBJECT(radio_medium),"clicked",
		GTK_SIGNAL_FUNC(radio_action),NULL);
	g_signal_connect(GTK_OBJECT(radio_deep),"clicked",
		GTK_SIGNAL_FUNC(radio_action),NULL);
	g_signal_connect(GTK_OBJECT(radio_selfdef),"clicked",
		GTK_SIGNAL_FUNC(radio_action),NULL);

	g_signal_connect(GTK_OBJECT(slider_threshold),"value_changed",
		GTK_SIGNAL_FUNC(slider_threshold_action),NULL);
	g_signal_connect(GTK_OBJECT(edit_threshold),"activate",
		GTK_SIGNAL_FUNC(edit_threshold_action),NULL);
	g_signal_connect(GTK_OBJECT(checkbox_auto_refresh),"clicked",
		GTK_SIGNAL_FUNC(checkbox_auto_refresh_action),NULL);

	g_signal_connect(GTK_OBJECT(OK_button),"clicked",
		GTK_SIGNAL_FUNC(OK_action),NULL);
	g_signal_connect(GTK_OBJECT(Cancel_button),"clicked",
		GTK_SIGNAL_FUNC(Cancel_action),NULL);

	//g_signal_connect(G_OBJECT(window_setting),"delete_event",G_CALLBACK(delete_event), NULL);
	//gtk_window_set_modal(GTK_WINDOW(window_setting), TRUE);
	gtk_container_add(GTK_CONTAINER(window_setting), vbox);
	gtk_widget_show(vbox);
	setting_window_init();

	setting_window_text_change();

	return window_setting;
}

#ifdef SIMULATION
int main( int argc, char *argv[])
{
    gtk_init(&argc, &argv);
    window_setting = create_setting_window();
    gtk_widget_show_all(window_setting);
    gtk_main();

    return 0;
}
#endif
