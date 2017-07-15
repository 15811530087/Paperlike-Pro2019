#ifndef __MANAGE_WINDOW_SHOW_HIDE_H__
#define __MANAGE_WINDOW_SHOW_HIDE_H__

extern GtkWidget *main_window;
extern GtkWidget *setting_window;
extern GtkWidget *help_window;
extern GtkWidget *about_window;

void main_window_text_change();
void setting_window_text_change();
void help_window_text_change();

void setting_window_delete_event();
void setting_window_create_event();
void help_window_create_event();
void help_window_delete_event();
void about_window_create_event();
void about_window_delete_event();

#endif
