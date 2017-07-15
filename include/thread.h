#ifndef __THREAD_H__
#define __THREAD_H__
#include "params.h"

extern unsigned char work_mode, show_mode;
extern unsigned short threshold;
extern unsigned short auto_clear_blur_time;
extern unsigned short screen_save_detect_time;
extern unsigned char screen_save_detect_enable_flag;
extern unsigned short screen_save_count;
extern unsigned short clear_blur_count;
extern unsigned char auto_clear_blur_enable;
extern unsigned char soft_hand_refresh_enable;
extern unsigned char hard_hand_refresh_enable;
extern unsigned char ResCommMode;
extern unsigned char language;


//flag
extern bool MonitorOnline;
extern unsigned char clear_blur_flag;
extern unsigned char screen_save_flag;
extern unsigned char set_threshold_flag;
extern unsigned short set_threshold_value;
extern unsigned char error_count;
extern unsigned char pause_flag;
extern unsigned char set_mode_flag;
extern unsigned char screen_saved_flag;
extern unsigned char resolution_change_flag;
extern struct monitor* global_DSmonitor;

int create_background_thread(void);

void restoreParameters(void);
void saveParameters(void);

#endif
