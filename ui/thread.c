#include <stdio.h>  
#include <string.h>
#include <stdlib.h>
#include <pthread.h>  
#include <unistd.h>  
#include "hotplug_monitor.h"
#include "ddcci.h"
#include "params.h"

pthread_t heart_thread_instance;
pthread_t screen_save_thread_instance;
pthread_t screen_blur_clear_thread_instance;
pthread_t kbd_instance;
pthread_t mouse_instance;
pthread_t mouse_wheel_instance;

//global value
unsigned char work_mode = 1, show_mode = 1, ResCommMode = DS_RES_HIGH;
unsigned short threshold = 156;
unsigned short auto_clear_blur_time = 120;
unsigned short screen_save_detect_time = 10;
unsigned char screen_save_detect_enable_flag = 1;
unsigned short screen_save_count = 0;
unsigned short clear_blur_count = 0;
unsigned char auto_clear_blur_enable = 0;
unsigned char soft_hand_refresh_enable = 0;
unsigned char hard_hand_refresh_enable = 0;
unsigned char language = LANG_ZH;

//flag
volatile bool MonitorOnline = false;
unsigned char clear_blur_flag = 0;
unsigned char screen_save_flag = 0;
unsigned char set_threshold_flag = 0;
unsigned char resolution_change_flag = 0;
unsigned short set_threshold_value = 150;
unsigned char error_count = 0;
unsigned char pause_flag = 0;
unsigned char set_mode_flag = 0;
unsigned char screen_saved_flag = 0;
struct monitor* global_DSmonitor = NULL;

extern void *detect_keyboard_event(void *param);
extern void *mouse_event(void *param);
extern void *mouse_wheel_click(void *param);
extern char ui_changed_flag;
extern char setting_ui_changed_flag;

void restoreParameters(void)
{
    char UserDataFile[48];
    char data[128];
    char *ptr;
    FILE *file;

    snprintf(UserDataFile, 48, "%s/.PaperLike", getenv("HOME"));
    DSPRINT("local user private data file is %s.\n", UserDataFile);

    file = fopen(UserDataFile, "r");
    if (file == NULL)
        return;
    
    fseek(file, 0, SEEK_SET);

    memset(data, 0, 128);
    ptr = fgets(data, sizeof(data), file);
    if (ptr == NULL) {
        fclose(file);
        DSPRINT("configure is null.\n");
        return;
    }
    auto_clear_blur_enable = atoi(data);
    DSPRINT("auto_clear_blur_enable(%d)\n", auto_clear_blur_enable);
    
    memset(data, 0, 128);
    ptr = fgets(data, sizeof(data), file);
    if (ptr == NULL) {
        fclose(file);
        DSPRINT("configure file is end.\n");
        return;
    }
    soft_hand_refresh_enable = atoi(data);
    DSPRINT("soft_hand_refresh_enable(%d)\n", soft_hand_refresh_enable);
    
    memset(data, 0, 128);
    ptr = fgets(data, sizeof(data), file);
    if (ptr == NULL) {
        fclose(file);
        DSPRINT("configure file is end.\n");
        return;
    }
    hard_hand_refresh_enable = atoi(data);
    DSPRINT("hard_hand_refresh_enable(%d)\n", hard_hand_refresh_enable);
    
    memset(data, 0, 128);
    ptr = fgets(data, sizeof(data), file);
    if (ptr == NULL) {
        fclose(file);
        DSPRINT("configure file is end.\n");
        return;
    }
    auto_clear_blur_time = atoi(data);
    DSPRINT("auto_clear_blur_time(%d)\n", auto_clear_blur_time);
    
    memset(data, 0, 128);
    ptr = fgets(data, sizeof(data), file);
    if (ptr == NULL) {
        fclose(file);
        DSPRINT("configure file is end.\n");
        return;
    }
    screen_save_detect_time = atoi(data);
    DSPRINT("screen_save_detect_time(%d)\n", screen_save_detect_time);
    
    memset(data, 0, 128);
    ptr = fgets(data, sizeof(data), file);
    if (ptr == NULL) {
        fclose(file);
        DSPRINT("configure file is end.\n");
        return;
    }
    show_mode = atoi(data);
    DSPRINT("show_mode(%d)\n", show_mode);
    
    memset(data, 0, 128);
    ptr = fgets(data, sizeof(data), file);
    if (ptr == NULL) {
        fclose(file);
        DSPRINT("configure file is end.\n");
        return;
    }
    work_mode = atoi(data);
    DSPRINT("work_mode(%d)\n", work_mode);
    
    memset(data, 0, 128);
    ptr = fgets(data, sizeof(data), file);
    if (ptr == NULL) {
        fclose(file);
        DSPRINT("configure file is end.\n");
        return;
    }
    ResCommMode = atoi(data);
    DSPRINT("ResCommMode (%d)\n", ResCommMode);
    
    memset(data, 0, 128);
    ptr = fgets(data, sizeof(data), file);
    if (ptr == NULL) {
        fclose(file);
        DSPRINT("configure file is end.\n");
        return;
    }
    threshold = atoi(data);
    DSPRINT("Threshold (%d)\n", threshold);
    
    memset(data, 0, 128);
    ptr = fgets(data, sizeof(data), file);
    if (ptr == NULL) {
        fclose(file);
        DSPRINT("configure file is end.\n");
        return;
    }
    language = atoi(data);
    DSPRINT("Language (%d)\n", language);
}

void saveParameters(void)
{
    char UserDataFile[48];
    char data[128];
    FILE *file;
    snprintf(UserDataFile, 48, "%s/.PaperLike", getenv("HOME"));
    DSPRINT("local user private data file is %s.\n", UserDataFile);
    
    file = fopen(UserDataFile, "w");
    if (file == NULL)
        return;

    memset(data, 0, 128);
    snprintf(data, 128, "%d\n", auto_clear_blur_enable);
    fputs(data, file);
    
    memset(data, 0, 128);
    snprintf(data, 128, "%d\n", soft_hand_refresh_enable);
    fputs(data, file);
    
    memset(data, 0, 128);
    snprintf(data, 128, "%d\n", hard_hand_refresh_enable);
    fputs(data, file);
    
    memset(data, 0, 128);
    snprintf(data, 128, "%d\n", auto_clear_blur_time);
    fputs(data, file);
    
    memset(data, 0, 128);
    snprintf(data, 128, "%d\n", screen_save_detect_time);
    fputs(data, file);
       
    memset(data, 0, 128);
    snprintf(data, 128, "%d\n", show_mode);
    fputs(data, file);
    
    memset(data, 0, 128);
    snprintf(data, 128, "%d\n", work_mode);
    fputs(data, file);
    
    memset(data, 0, 128);
    snprintf(data, 128, "%d\n", ResCommMode);
    fputs(data, file);
    
    memset(data, 0, 128);
    snprintf(data, 128, "%d\n", threshold);
    fputs(data, file);
    
    memset(data, 0, 128);
    snprintf(data, 128, "%d\n", language);
    fputs(data, file);
    
    fclose(file);
}


bool detect_monitor(void)
{
	struct monitorlist* monlist;
	struct monitor* mon;

	//if (monitor_hotplug_in == 0)
	//	return false;

	monitor_hotplug_in = 0;
	monlist = ddcci_probe();

	if (monlist) {
		mon = ddcci_detect_DSMonitor(monlist);
		if (global_DSmonitor) {
			ddcci_close(global_DSmonitor);
			free(global_DSmonitor);
			global_DSmonitor = NULL;
		}
		if (mon)
			DSPRINT("detected DS monitor.\n");
		global_DSmonitor = mon;

		ddcci_free_list(monlist);

		if (mon)
			return true;
	}

	return false;
}

void *screen_save_thread(void *param)
{
	while (1) {
		if (MonitorOnline == false) {
			sleep(1);
			continue;
		}

		if (screen_save_detect_enable_flag == 0) {
			sleep(1);
			screen_save_count = 0;
			continue;
		}

		sleep(1);

		DSPRINT("screen_save_count is %d.\n", screen_save_count);

		if (screen_save_count >= screen_save_detect_time) {
			screen_save_flag = 1;
			DSPRINT("raise up screen save flag.\n");
			continue;
		}

		if (screen_save_count == 0)
			screen_save_flag = 0;

		screen_save_count++;
	}
}

void *screen_blur_clear_thread(void *param)
{
	static int blur_count = 0;

	while (1) {
		if ((MonitorOnline == false) ||
			(auto_clear_blur_enable == 0) ||
			(screen_saved_flag == 1)) {
			sleep(1);
			continue;
		}

		sleep(1);
		clear_blur_count++;

		if (auto_clear_blur_time <= clear_blur_count) {
			clear_blur_flag = 2;
			clear_blur_count = 0;
		}
	}
}

unsigned short convert_to_threshold(unsigned short step)
{
    unsigned short result = 0;
    
    switch (show_mode) {
        case DS_MODE_A2:
        case DS_MODE_LOW_A2:
            //130 - 250, default 190
            result = 130 + (step - 1) * 15;
            break;
        case DS_MODE_FLOYD:
            //1 - 9, default 5
            result = 28 + (step - 1) * 28;
            break;
        case DS_MODE_LOW_A5:
            //80 - 200 default 160
            result = 80 + (step - 1) * 15;
            break;
    }
    
    DSPRINT("***** after calculate, threshold result is %d .\n", result);
    return result;
}


unsigned short threshold_calculate(unsigned short temp_threshold)
{
    unsigned short result = 0;
    
    switch (show_mode) {
        case DS_MODE_A2:
        case DS_MODE_LOW_A2:
            //130 - 250, default 190
            if (temp_threshold <= 130)
                result = 130;
            else if (temp_threshold >= 250)
                result = 250;
            else {
                result = temp_threshold;
            }
            break;
        case DS_MODE_FLOYD:
            //1 - 9, default 5
            if (temp_threshold <= 28)
                result = 1;
            else if (temp_threshold <= 56)
                result = 2;
            else if (temp_threshold <= 84)
                result = 3;
            else if (temp_threshold <= 112)
                result = 4;
            else if (temp_threshold <= 140)
                result = 5;
            else if (temp_threshold <= 168)
                result = 6;
            else if (temp_threshold <= 196)
                result = 7;
            else if (temp_threshold <= 224)
                result = 8;
            else
                result = 9;
            break;
        case DS_MODE_LOW_A5:
            //80 - 200 default 160
            if (temp_threshold <= 80)
                result = 80;
            else if (temp_threshold >= 200)
                result = 200;
            else {
                result = temp_threshold;
            }
            break;
    }
    
    DSPRINT("***** after calculate, threshold result is %d .\n", result);
    return result;
}

void *heart_thread(void *param)
{
	bool res = false;
	unsigned char replay_data[11];
	unsigned char replay_size = 8;
	char packet_type;
	char packet_data;
	char screen_saver_heart_count = 0;


	if (!ddcci_init(NULL)) {
		DSPRINT("Unable to initialize ddcci library.\n");
		return NULL;
	}

	while (1) {
		if (pause_flag == 1) {
			sleep(1);
			continue;
		}

		if (error_count >= ERROR_THRESH_COUNT) {
			DSPRINT("I2C communication error count exceed threshold.\n");
			if (global_DSmonitor) {
				ddcci_close(global_DSmonitor);
				free(global_DSmonitor);
				global_DSmonitor = NULL;
			}
			MonitorOnline = false;
			error_count = 0;
			sleep(8);
		}
		if (MonitorOnline == false) {
			res = detect_monitor();
			if (res == true) {
				sleep(5);
				//send_screen_save_flag(global_DSmonitor, 0);
				send_mode_selection(global_DSmonitor, show_mode, work_mode);
				usleep(200*1000);
				send_monitor_threshold(global_DSmonitor, threshold);
				usleep(200*1000);
				send_clear_monitor(global_DSmonitor, 1);
				usleep(200*1000);
				//main_window_init();
				MonitorOnline = true;
				//ui_changed_flag = 1;
				/*switch (ResCommMode) {
                    		case DS_RES_LOW:
                        		SendMonitorRes(global_DSmonitor, DS_RES_800_X_600);
                        	break;
                    		case DS_RES_HIGH:
                        		SendMonitorRes(global_DSmonitor, DS_RES_OTHER);
                        	break;
                		}*/
				//sleep(5);
				ui_changed_flag = 1;
				setting_ui_changed_flag = 1;
				continue;
			} else {
				//sleep 10s
				sleep(3);
				continue;
			}
		}
		if (screen_save_flag) {
			if (screen_saved_flag == 0) {
				screen_saved_flag = 1;
				send_screen_save_flag(global_DSmonitor, 1);
			}
		} else {
			if (screen_saved_flag == 1) {
				screen_saved_flag = 0;
				send_screen_save_flag(global_DSmonitor, 0);
			}
		}
		if (screen_saved_flag == 1) {
			screen_saver_heart_count++;
			if (screen_saver_heart_count >= 50) {
				screen_saver_heart_count = 0;
				send_screen_save_flag(global_DSmonitor, 1);
			}
		} else {
			screen_saver_heart_count = 0;
		}

		switch (clear_blur_flag) {
		case 1:
			send_clear_monitor(global_DSmonitor, 1);
			clear_blur_flag = 0;
			break;
		case 2:
			send_clear_monitor(global_DSmonitor, 0);
			clear_blur_flag = 0;
			break;
		}

		if (set_threshold_flag) {
			set_threshold_flag = 0;
			usleep(200*1000);
			//send_monitor_threshold(global_DSmonitor, set_threshold_value);
			unsigned short d = threshold_calculate(set_threshold_value);
            		DSPRINT("current mode is %d. d as %d\n", show_mode, d);
            		switch (show_mode) {
                	case DS_MODE_FLOYD:
                    		SendMonitorFloydThreshold(global_DSmonitor, d);
                    	break;
                	case DS_MODE_LOW_A5:
                    		SendMonitorLowA5Threshold(global_DSmonitor, d);
                    	break;
                	case DS_MODE_LOW_A2:
                    		SendMonitorLowA2Threshold(global_DSmonitor, d);
                    	break;
                	case DS_MODE_A2:
                    		SendMonitorA2Threshold(global_DSmonitor, d);
                    	break;
                	default:
                    	break;
            		}
		}

		if (set_mode_flag) {
			set_mode_flag = 0;
			send_mode_selection(global_DSmonitor, show_mode, work_mode);
			switch (show_mode) {
                	case DS_MODE_FLOYD:
                    		set_threshold_value = DS_MODE_DEF_THRESHOLD_FLOYD;
                    		threshold = DS_MODE_DEF_THRESHOLD_FLOYD;
                    		set_threshold_flag = 1;
                    	break;
                	case DS_MODE_LOW_A5:
                    		set_threshold_value = DS_MODE_DEF_THRESHOLD_A5;
                    		threshold = DS_MODE_DEF_THRESHOLD_A5;
                    		set_threshold_flag = 1;
                    	break;
                	case DS_MODE_LOW_A2:
                	case DS_MODE_A2:
                    		set_threshold_value = DS_MODE_DEF_THRESHOLD_A2;
                    		threshold = DS_MODE_DEF_THRESHOLD_A2;
                    		set_threshold_flag = 1;
                    	break;
            		}
		}

		if (resolution_change_flag) {
			resolution_change_flag = 0;
			usleep(300*1000);
			set_threshold_flag = 1;
			switch (ResCommMode) {
			case DS_RES_LOW:
				DSPRINT("--------------  send monitor resolution.\n");
				SendMonitorRes(global_DSmonitor, DS_RES_800_X_600);
				break;
			case DS_RES_HIGH:
				SendMonitorRes(global_DSmonitor, DS_RES_OTHER);
				break;
			}
			usleep(300*1000);
		}

		if (send_heart_signal(global_DSmonitor, true) == false)
			error_count++;

		usleep(100*1000);
		memset(replay_data, 0, 8);
		res = DSMonitorReadData(global_DSmonitor, NULL, 0, replay_data, replay_size);
		DSPRINT("heart return status : %.02x, %.02x, %.02x, %.02x.\n", replay_data[4],
			replay_data[5], replay_data[6], replay_data[7]);
		if (res == false) {
			error_count++;
			continue;
		}
		packet_type = replay_data[5];
		packet_data = replay_data[7] & 0xf;

		switch (packet_type) {
		case 0x07:
		{
			if ((packet_data >= 1) && (packet_data <= 6)) {
				if (ResCommMode == DS_RES_LOW)
					show_mode = packet_data + 3;
				else
					show_mode = packet_data;
				ui_changed_flag = 1;
				setting_ui_changed_flag = 1;
			} else if (packet_data == 8) {
				//screen_save_count = 0;
				//screen_save_flag = 0;
			}
			screen_save_count = 0;
			screen_save_flag = 0;
			break;
		}
		case 0x01:
			if (packet_data == 0x2)
				DSPRINT("Heart Ok.\n");
			break;
		case 0x8:
		case 0x9:
		case 0xb:
			threshold = convert_to_threshold(packet_data);
			screen_save_count = 0;
			screen_save_flag = 0;
			setting_ui_changed_flag = 1;
			break;
		}
		usleep(200*1000);
		//sleep(1);
	}

	ddcci_release();

	return NULL;
}

int create_background_thread(void)
{
	int ret;
	ret = pthread_create(&heart_thread_instance, NULL,
			heart_thread, NULL);
	if (ret)
		perror("create heart thread failed.\n");

	ret = pthread_create(&screen_save_thread_instance, NULL,
			screen_save_thread, NULL);
	if (ret)
		perror("create screen save thread failed.\n");

	ret = pthread_create(&screen_blur_clear_thread_instance, NULL,
			screen_blur_clear_thread, NULL);
	if (ret)
		perror("create screen blur clear thread failed.\n");

	ret = pthread_create(&kbd_instance, NULL,
			detect_keyboard_event, NULL);
	if (ret)
		perror("create kbd event failed.\n");

	ret = pthread_create(&mouse_instance, NULL,
			mouse_event, NULL);
	if (ret)
		perror("create mouse event failed.\n");

	ret = pthread_create(&mouse_wheel_instance, NULL,
			mouse_wheel_click, NULL);
	if (ret)
		perror("create mouse event failed.\n");

	return ret;
}


