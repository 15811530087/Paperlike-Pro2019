/*
    ddc/ci interface functions header
    Copyright(c) 2004 Oleg I. Vdovikin (oleg@cs.msu.su)
    Copyright(c) 2004-2005 Nicolas Boichat (nicolas@boichat.ch)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#ifndef DDCCI_H
#define DDCCI_H

#define _(String) String
#define N_(String) String

#include <time.h>
#include <sys/time.h>

#include "params.h"

struct profile;

enum monitor_type {
unk = 0,
lcd = 1,
crt = 2
};

/* Structure to store CAPS vcp entry (control and related values) */
struct vcp_entry {
	int values_len; /* -1 if values are not specified */
	unsigned short* values;
};

/* Structure to store CAPS */
struct caps {
	struct vcp_entry* vcp[256]; /* vcp entries */
	enum monitor_type type;
	char* raw_caps; /* raw text caps */
};

struct monitor {
	int fd;
	unsigned int addr;
	int adl_adapter, adl_display;
	char pnpid[8];
	unsigned char digital; /* 0 - digital, 1 - analog */
	struct timeval last;
	struct caps caps;
	
	enum {
		dev
#ifdef HAVE_DDCPCI
		,pci
#endif
		,type_adl
	} type;
	int probing; /* are we probing? */
	
	int fallback;
	int DSMonitor;
	/* 0 - the db is designed for this monitor
	   1 - we are using a manufacturer standard profile (warn the user)
	   2 - we are using the VESA generic profile (warn the user) */
};

/* Struct used to return monitor data probed by ddcci_probe */
struct monitorlist {
	char* filename; /* I2C device filename */
	
	unsigned char supported; /* 0 - DDC/CI not supported, 1 - DDC/CI supported */
	char* name;
	unsigned char digital; /* 0 - digital, 1 - analog */
	unsigned char DSMonitor;
	
	struct monitorlist* next;
};

struct monitorlist* ddcci_probe();
void ddcci_free_list(struct monitorlist* list);

int ddcci_open(struct monitor* mon, const char* filename, int probing);
int ddcci_save(struct monitor* mon);
int ddcci_close(struct monitor* mon);

int ddcci_writectrl(struct monitor* mon, unsigned char ctrl, unsigned short value, int delay);

/* return values: < 0 - failure, 0 - contron not supported, > 0 - supported */
int ddcci_readctrl(struct monitor* mon, unsigned char ctrl, 
	unsigned short *value, unsigned short *maximum);

int ddcci_parse_caps(const char* caps_str, struct caps* caps, int add);

int ddcci_caps(struct monitor* mon);

/* verbosity level (0 - normal, 1 - encoded data, 2 - ddc/ci frames) */
void ddcci_verbosity(int verbosity);

int get_verbosity();

int ddcci_init(char* usedatadir);

void ddcci_release();

struct monitor* ddcci_detect_DSMonitor(struct monitorlist* mlist);

void ddcpci_send_heartbeat();

/* Create $HOME/.ddccontrol and subdirectories if necessary */
int ddcci_create_config_dir();

bool DSMonitorWriteData(struct monitor *mon, unsigned char *data, unsigned char dataSize, bool needACK);

bool DSMonitorReadData(struct monitor *mon, unsigned char *data, unsigned char dataSize,
                       unsigned char *replayBuf, unsigned char replaySize);

bool send_screen_save_flag(struct monitor* mon, unsigned char flag);

bool send_mode_selection(struct monitor* mon, unsigned char show_mode,
	unsigned char work_mode);

bool SendMonitorRes(struct monitor* mon, unsigned char ResMode);

bool send_monitor_threshold(struct monitor* mon, unsigned short threshold);

bool send_clear_monitor(struct monitor* mon, unsigned short soft_hard);

bool send_heart_signal(struct monitor* mon, bool heart);

bool SendMonitorFloydThreshold(struct monitor* mon, unsigned short Threshold);

bool SendMonitorA2Threshold(struct monitor* mon, unsigned short Threshold);

bool SendMonitorLowA2Threshold(struct monitor* mon, unsigned short Threshold);

bool SendMonitorLowA5Threshold(struct monitor* mon, unsigned short Threshold);

bool SendMonitorAdvancedSpeed(struct monitor* mon, unsigned char SpeedLevel);

/* Macros */
#define DDCCI_DB_RETURN_IF_RUN(cond, value, message, node, run) \
	if (cond) { \
		if (node) \
			fprintf(stderr, _("Error: %s @%s:%d (%s:%ld)\n"), message, __FILE__, __LINE__, \
				((xmlNodePtr)node)->doc->name, XML_GET_LINE(node)); \
		else \
			fprintf(stderr, _("Error: %s @%s:%d\n"), message, __FILE__, __LINE__); \
		run \
		return value; \
	}

#define DDCCI_DB_RETURN_IF(cond, value, message, node) \
	DDCCI_DB_RETURN_IF_RUN(cond, value, message, node, {})
	
#define DDCCI_RETURN_IF_RUN(cond, value, message, run) \
	if (cond) { \
		fprintf(stderr, _("Error: %s @%s:%d\n"), message, __FILE__, __LINE__); \
		run \
		return value; \
	}

#define DDCCI_RETURN_IF(cond, value, message) \
	DDCCI_RETURN_IF_RUN(cond, value, message, {})

#endif //DDCCI_H
