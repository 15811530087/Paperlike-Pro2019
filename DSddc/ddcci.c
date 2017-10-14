/*
    ddc/ci interface functions
    Copyright(c) 2004 Oleg I. Vdovikin (oleg@cs.msu.su)
    Copyright(c) 2004-2006 Nicolas Boichat (nicolas@boichat.ch)

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

#include <errno.h>
#include <stdio.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>

#include "i2c-dev.h"
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "ddcci.h"
#include "amd_adl.h"
#include "params.h"

/* ddc/ci defines */
#define DEFAULT_DDCCI_ADDR	0x37	/* ddc/ci logic sits at 0x37 */
#define DEFAULT_EDID_ADDR	0x50	/* edid sits at 0x50 */

#define DDCCI_COMMAND_READ	0x01	/* read ctrl value */
#define DDCCI_REPLY_READ	0x02	/* read ctrl value reply */
#define DDCCI_COMMAND_WRITE	0x03	/* write ctrl value */

#define DDCCI_COMMAND_SAVE	0x0c	/* save current settings */

#define DDCCI_REPLY_CAPS	0xe3	/* get monitor caps reply */
#define DDCCI_COMMAND_CAPS	0xf3	/* get monitor caps */
#define DDCCI_COMMAND_PRESENCE	0xf7	/* ACCESS.bus presence check */

/* control numbers */
#define DDCCI_CTRL_BRIGHTNESS	0x10

/* samsung specific, magictune starts with writing 1 to this register */
#define DDCCI_CTRL		0xf5
#define DDCCI_CTRL_ENABLE	0x0001
#define DDCCI_CTRL_DISABLE	0x0000

/* ddc/ci iface tunables */
#define MAX_BYTES		127	/* max message length */
#define DELAY   		45000	/* uS to wait after write */

#define CONTROL_WRITE_DELAY   80000	/* uS to wait after writing to a control (default) */

/* magic numbers */
#define MAGIC_1	0x51	/* first byte to send, host address */
#define MAGIC_2	0x80	/* second byte to send, ored with length */
#define MAGIC_XOR 0x50	/* initial xor for received frame */

/* verbosity level (0 - normal, 1 - encoded data, 2 - ddc/ci frames) */
static int verbosity = 0;

void ddcci_verbosity(int _verbosity)
{
	verbosity = _verbosity;
}

int get_verbosity() {
	return verbosity;
}

/* debugging */
static void dumphex(FILE *f, char *text, unsigned char *buf, int len)
{
	int i, j;

	if (text) {
		if (len > 16) {
			fprintf(f, "%s:\n", text);
		}
		else {
			fprintf(f, "%s: ", text);
		}
	}

	for (j = 0; j < len; j +=16) {
		if (len > 16) {
			fprintf(f, "%04x: ", j);
		}
		
		for (i = 0; i < 16; i++) {
			if (i + j < len) fprintf(f, "%02x ", buf[i + j]);
			else fprintf(f, "   ");
		}

		fprintf(f, "| ");

		for (i = 0; i < 16; i++) {
			if (i + j < len) fprintf(f, "%c", 
				buf[i + j] >= ' ' && buf[i + j] < 127 ? buf[i + j] : '.');
			else fprintf(f, " ");
		}
		
		fprintf(f, "\n");
	}
}

/* IPC functions */
#ifdef HAVE_DDCPCI
#include "ddcpci-ipc.h"
#include <sys/msg.h>
#include <sys/ipc.h>

#define DDCPCI_RETRY_DELAY 10000 /* in us */
#define DDCPCI_RETRIES 100000

static int msqid = -2;

int ddcpci_init()
{
	if (msqid == -2) {
		if (verbosity) {
			DSPRINT("ddcpci initing...\n");
		}
		
		key_t key = ftok(BINDIR "/ddcpci", getpid());
		
		if ((msqid = msgget(key, IPC_CREAT | 0666)) < 0) {
			perror(_("Error while initialisating the message queue"));
			return 0;
		}
		
		char buffer[256];
		
		snprintf(buffer, 256, BINDIR "/ddcpci %d %d &", verbosity, key);
		
		if (verbosity) {
			DSPRINT("Starting %s...\n", buffer);
		}
		
		system(buffer);
	}
	return (msqid >= 0);
}

void ddcpci_release()
{
	if (verbosity) {
		DSPRINT("ddcpci being released...\n");
	}
	if (msqid >= 0) {
		struct query qlist;
		memset(&qlist, 0, sizeof(struct query));
		qlist.mtype = 1;
		qlist.qtype = QUERY_QUIT;
		
		if (msgsnd(msqid, &qlist, QUERY_SIZE, IPC_NOWAIT) < 0) {
			perror(_("Error while sending quit message"));
		}
		
		usleep(20000);
		
		msgctl(msqid, IPC_RMID, NULL);
	}
}

/* Returns : 0 - OK, negative value - timed out or another error */
int ddcpci_read(struct answer* manswer)
{
	int i, ret;
	for (i = DDCPCI_RETRIES;; i--) {
		if ((ret = msgrcv(msqid, manswer, sizeof(struct answer) - sizeof(long), 2, IPC_NOWAIT)) < 0) {
			if (errno != ENOMSG) {
				return -errno;
			}
		}
		else {
			if (manswer->status < 0) {
				errno = EBADMSG;
				return -EBADMSG;
			}
			else {
				return ret;
			}
		}
		
		if (i == 0) {
			errno = ETIMEDOUT;
			return -ETIMEDOUT;
		}
		usleep(DDCPCI_RETRY_DELAY);
	}
}

/* Send heartbeat so ddcpci doesn't timeout */
void ddcpci_send_heartbeat() {
	if (msqid >= 0) {
		struct query qheart;
		memset(&qheart, 0, sizeof(struct query));
		qheart.mtype = 1;
		qheart.qtype = QUERY_HEARTBEAT;
		
		if (msgsnd(msqid, &qheart, QUERY_SIZE, IPC_NOWAIT) < 0) {
			perror(_("Error while sending heartbeat message"));
		}
	}
}

#else
int ddcpci_init() {
	return 1;
}

void ddcpci_release() {}

void ddcpci_send_heartbeat() {}
#endif

int ddcci_init(char* usedatadir)
{
	if (!amd_adl_init()){
		if (verbosity) {
			DSPRINT(_("Failed to initialize ADL...\n"));
		}
	}
	return 1;
}

void ddcci_release() {
	amd_adl_free();
}

/* write len bytes (stored in buf) to i2c address addr */
/* return 0 on success, -1 on failure */
static int i2c_write(struct monitor* mon, unsigned int addr, unsigned char *buf, unsigned char len)
{
	switch (mon->type) {
	case dev:
	{	
		int i;
		struct i2c_rdwr_ioctl_data msg_rdwr;
		struct i2c_msg             i2cmsg;
	
		/* done, prepare message */	
		msg_rdwr.msgs = &i2cmsg;
		msg_rdwr.nmsgs = 1;
	
		i2cmsg.addr  = addr;
		i2cmsg.flags = 0;
		i2cmsg.len   = len;
		i2cmsg.buf   = buf;
	
		if ((i = ioctl(mon->fd, I2C_RDWR, &msg_rdwr)) < 0 )
		{
			if (!mon->probing || verbosity) {
				perror("ioctl()");
				fprintf(stderr,_("ioctl returned %d\n"),i);
			}
			return -1;
		}

		if (verbosity > 1) {
			dumphex(stderr, "Send", buf, len);
		}

		return i;
	}
#ifdef HAVE_DDCPCI
	case pci:
	{
		struct query qdata;
		memset(&qdata, 0, sizeof(struct query));
		qdata.mtype = 1;
		qdata.qtype = QUERY_DATA;
		qdata.addr = addr;
		qdata.flags = 0;
		memcpy(qdata.buffer, buf, len);
		
		if (msgsnd(msqid, &qdata, QUERY_SIZE + len, IPC_NOWAIT) < 0) {
			if (!mon->probing || verbosity) {
				perror(_("Error while sending write message"));
			}
			return -3;
		}
		
		struct answer adata;
		
		if (ddcpci_read(&adata) < 0) {
			if (!mon->probing || verbosity) {
				perror(_("Error while reading write message answer"));
			}
			return -1;
		}

		if (verbosity > 1) {
			dumphex(stderr, "Send", buf, len);
		}

		return adata.status;
	}
#endif
	case type_adl:
	{
		return amd_adl_i2c_write(mon->adl_adapter, mon->adl_display, addr, buf, len);
	}
	default:
		return -1;
	}
}

/* read at most len bytes from i2c address addr, to buf */
/* return -1 on failure */
static int i2c_read(struct monitor* mon, unsigned int addr, unsigned char *buf, unsigned char len)
{
	switch (mon->type) {
	case dev:
	{
		struct i2c_rdwr_ioctl_data msg_rdwr;
		struct i2c_msg             i2cmsg;
		int i;
	
		msg_rdwr.msgs = &i2cmsg;
		msg_rdwr.nmsgs = 1;
	
		i2cmsg.addr  = addr;
		i2cmsg.flags = I2C_M_RD;
		i2cmsg.len   = len;
		i2cmsg.buf   = buf;
	
		if ((i = ioctl(mon->fd, I2C_RDWR, &msg_rdwr)) < 0)
		{
			if (!mon->probing || verbosity) {
				perror("ioctl()");
				fprintf(stderr,_("ioctl returned %d\n"),i);
			}
			return -1;
		}

		if (verbosity > 1) {
			dumphex(stderr, "Recv", buf, i);
		}

		return i;
	}
#ifdef HAVE_DDCPCI
	case pci:
	{
		int ret;
		struct query qdata;
		memset(&qdata, 0, sizeof(struct query));
		qdata.mtype = 1;
		qdata.qtype = QUERY_DATA;
		qdata.addr = addr;
		qdata.flags = I2C_M_RD;
		qdata.len = len;
		
		if (msgsnd(msqid, &qdata, QUERY_SIZE, IPC_NOWAIT) < 0) {
			if (!mon->probing || verbosity) {
				perror(_("Error while sending read message"));
			}
			return -3;
		}
		
		struct answer adata;
		
		if ((ret = ddcpci_read(&adata)) < 0) {
			if (!mon->probing || verbosity) {
				perror(_("Error while reading read message answer"));
			}
			return -1;
		}
		
		memcpy(buf, adata.buffer, ret - ANSWER_SIZE);
		
		if (verbosity > 1) {
			dumphex(stderr, "Recv", buf, ret - ANSWER_SIZE);
		}

		return ret - ANSWER_SIZE;
	}
#endif
	case type_adl:
	{
		return amd_adl_i2c_read(mon->adl_adapter, mon->adl_display, addr, buf, len);
	}
	default:
		return -1;
	}
}

/* stalls execution, allowing write transaction to complete */
static void ddcci_delay(struct monitor* mon, int iswrite)
{
	struct timeval now;

	if (gettimeofday(&now, NULL)) {
		usleep(DELAY);
	} else {
		if (mon->last.tv_sec >= (now.tv_sec - 1)) {
			unsigned long usec = (now.tv_sec - mon->last.tv_sec) * 10000000 +
				now.tv_usec - mon->last.tv_usec;

			if (usec < DELAY) {
				usleep(DELAY - usec);
				if ((now.tv_usec += (DELAY - usec)) > 1000000) {
					now.tv_usec -= 1000000;
					now.tv_sec++;
				}
			}
		}
		
		if (iswrite) {
			mon->last = now;
		}
	}
}

/* write len bytes (stored in buf) to ddc/ci at address addr */
/* return 0 on success, -1 on failure */
static int ddcci_write(struct monitor* mon, unsigned char *buf, unsigned char len)
{
	int i = 0;
	unsigned char _buf[MAX_BYTES + 3];
	unsigned xor = ((unsigned char)mon->addr << 1);	/* initial xor value */

	/* put first magic */
	xor ^= (_buf[i++] = MAGIC_1);
	
	/* second magic includes message size */
	xor ^= (_buf[i++] = MAGIC_2 | len);
	
	while (len--) /* bytes to send */
		xor ^= (_buf[i++] = *buf++);
		
	/* finally put checksum */
	_buf[i++] = xor;

	/* wait for previous command to complete */
	ddcci_delay(mon, 1);

	return i2c_write(mon, mon->addr, _buf, i);
}

/* read ddc/ci formatted frame from ddc/ci at address addr, to buf */
static int ddcci_read(struct monitor* mon, unsigned char *buf, unsigned char len)
{
	unsigned char _buf[MAX_BYTES];
	unsigned char xor = MAGIC_XOR;
	int i, _len;

	/* wait for previous command to complete */
	ddcci_delay(mon, 0);

	if (i2c_read(mon, mon->addr, _buf, len + 3) <= 0) /* busy ??? */
	{
		return -1;
	}
	
	/* validate answer */
	if (_buf[0] != mon->addr * 2) { /* busy ??? */
		if (!mon->probing || verbosity) {
			fprintf(stderr, _("Invalid response, first byte is 0x%02x, should be 0x%02x\n"),
				_buf[0], mon->addr * 2);
			dumphex(stderr, NULL, _buf, len + 3);
		}
		return -1;
	}

	if ((_buf[1] & MAGIC_2) == 0) {
		/* Fujitsu Siemens P19-2 and NEC LCD 1970NX send wrong magic when reading caps. */
		if (!mon->probing || verbosity) {
			fprintf(stderr, _("Non-fatal error: Invalid response, magic is 0x%02x\n"), _buf[1]);
		}
	}

	_len = _buf[1] & ~MAGIC_2;
	if (_len > len || _len > sizeof(_buf)) {
		if (!mon->probing || verbosity) {
			fprintf(stderr, _("Invalid response, length is %d, should be %d at most\n"),
				_len, len);
		}
		return -1;
	}

	/* get the xor value */
	for (i = 0; i < _len + 3; i++) {
		xor ^= _buf[i];
	}
	
	if (xor != 0) {
		if (!mon->probing || verbosity) {
			fprintf(stderr, _("Invalid response, corrupted data - xor is 0x%02x, length 0x%02x\n"), xor, _len);
			dumphex(stderr, NULL, _buf, len + 3);
		}
		
		return -1;
	}

	/* copy payload data */
	memcpy(buf, _buf + 2, _len);

	return _len;
}

/* write value to register ctrl of ddc/ci at address addr */
int ddcci_writectrl(struct monitor* mon, unsigned char ctrl, unsigned short value, int delay)
{
	unsigned char buf[4];

	buf[0] = DDCCI_COMMAND_WRITE;
	buf[1] = ctrl;
	buf[2] = (value >> 8);
	buf[3] = (value & 255);

	int ret = ddcci_write(mon, buf, sizeof(buf));
	
	/* Do the delay */
	if (delay > 0) {
		usleep(1000*delay);
	}
	/* Default delay : 80ms (anyway we won't get below 45ms (due to DELAY)) */
	else if (delay < 0) {
		usleep(CONTROL_WRITE_DELAY);
	}
	
	return ret;
}

/* read register ctrl raw data of ddc/ci at address addr */
static int ddcci_raw_readctrl(struct monitor* mon, 
	unsigned char ctrl, unsigned char *buf, unsigned char len)
{
	unsigned char _buf[2];

	_buf[0] = DDCCI_COMMAND_READ;
	_buf[1] = ctrl;

	if (ddcci_write(mon, _buf, sizeof(_buf)) < 0)
	{
		return -1;
	}

	return ddcci_read(mon, buf, len);
}

int ddcci_readctrl(struct monitor* mon, unsigned char ctrl, 
	unsigned short *value, unsigned short *maximum)
{
	unsigned char buf[8];

	int len = ddcci_raw_readctrl(mon, ctrl, buf, sizeof(buf));
	
	if (len == sizeof(buf) && buf[0] == DDCCI_REPLY_READ &&	buf[2] == ctrl) 
	{	
		if (value) {
			*value = buf[6] * 256 + buf[7];
		}
		
		if (maximum) {
			*maximum = buf[4] * 256 + buf[5];
		}
		
		return !buf[1];
		
	}
	
	return -1;
}

/* See documentation Appendix D.
 * Returns :
 * -1 if an error occured 
 *  number of controls added
 *
 * add: if true: add caps_str to caps, otherwise remove caps_str from the caps.
 */
int ddcci_parse_caps(const char* caps_str, struct caps* caps, int add)
{
//	printf("Parsing CAPS (%s).\n", caps_str);
	int pos = 0; /* position in caps_str */
	
	int level = 0; /* CAPS parenthesis level */
	int svcp = 0; /* Current CAPS section is vcp */
	int stype = 0; /* Current CAPS section is type */
	
	char buf[128];
	char* endptr;
	int ind = -1;
	long val = -1;
	int i;
	int removeprevious = 0;
	
	int num = 0;
	
	for (pos = 0; caps_str[pos] != 0; pos++)
	{
		if (caps_str[pos] == '(') {
			level++;
		}
		else if (caps_str[pos] == ')')
		{
			level--;
			if (level == 1) {
				svcp = 0;
				stype = 0;
			}
		}
		else if (caps_str[pos] != ' ')
		{
			if (level == 1) {
				if ((strncmp(caps_str+pos, "vcp(", 4) == 0) || (strncmp(caps_str+pos, "vcp ", 4) == 0)) {
					svcp = 1;
					pos += 2;
				}
				else if (strncmp(caps_str+pos, "type", 4) == 0) {
					stype = 1;
					pos += 3;
				}
			}
			else if ((stype == 1) && (level == 2)) {
				if (strncmp(caps_str+pos, "lcd", 3) == 0) {
					caps->type = lcd;
					pos += 2;
				}
				else if (strncmp(caps_str+pos, "crt", 3) == 0) {
					caps->type = crt;
					pos += 2;
				}
			}
			else if ((svcp == 1) && (level == 2)) {
				if (!add && ((removeprevious == 1) || (caps->vcp[ind] && caps->vcp[ind]->values_len == 0))) {
					if(caps->vcp[ind]) {
						if (caps->vcp[ind]->values) {
							free(caps->vcp[ind]->values);
						}
						free(caps->vcp[ind]);
						caps->vcp[ind] = NULL;
					}
				}
				buf[0] = caps_str[pos];
				buf[1] = caps_str[++pos];
				buf[2] = 0;
				ind = strtol(buf, &endptr, 16);
				if (*endptr != 0) {
					DSPRINT(_("Can't convert value to int, invalid CAPS (buf=%s, pos=%d).\n"), buf, pos);
					return -1;
				}
				if (add) {
					caps->vcp[ind] = malloc(sizeof(struct vcp_entry));
					caps->vcp[ind]->values_len = -1;
					caps->vcp[ind]->values = NULL;
				}
				else {
					removeprevious = 1;
				}
				num++;
			}
			else if ((svcp == 1) && (level == 3)) {
				i = 0;
				while ((caps_str[pos+i] != ' ') && (caps_str[pos+i] != ')')) {
					buf[i] = caps_str[pos+i];
					i++;
				}
				buf[i] = 0;
				val = strtol(buf, &endptr, 16);
				if (*endptr != 0) {
					DSPRINT(_("Can't convert value to int, invalid CAPS (buf=%s, pos=%d).\n"), buf, pos);
					return -1;
				}
				if (add) {
					if (caps->vcp[ind]->values_len == -1) {
						caps->vcp[ind]->values_len = 1;
					}
					else {
						caps->vcp[ind]->values_len++;
					}
					caps->vcp[ind]->values = realloc(caps->vcp[ind]->values, caps->vcp[ind]->values_len*sizeof(unsigned short));
					caps->vcp[ind]->values[caps->vcp[ind]->values_len-1] = val;
				}
				else {
					if (caps->vcp[ind]->values_len > 0) {
						removeprevious = 0;
						int j = 0;
						for (i = 0; i < caps->vcp[ind]->values_len; i++) {
							if (caps->vcp[ind]->values[i] != val) {
								caps->vcp[ind]->values[j++] = caps->vcp[ind]->values[i];
							}
						}
						caps->vcp[ind]->values_len--;
					}
				}
			}
		}
	}
	
	if (!add && ((removeprevious == 1) || (caps->vcp[ind] && caps->vcp[ind]->values_len == 0))) {
		if(caps->vcp[ind]) {
			if (caps->vcp[ind]->values) {
				free(caps->vcp[ind]->values);
			}
			free(caps->vcp[ind]);
			caps->vcp[ind] = NULL;
		}
	}
	
	return num;
}

/* read capabilities raw data of ddc/ci at address addr starting at offset to buf */
static int ddcci_raw_caps(struct monitor* mon, unsigned int offset, unsigned char *buf, unsigned char len)
{
	unsigned char _buf[3];

	_buf[0] = DDCCI_COMMAND_CAPS;
	_buf[1] = offset >> 8;
	_buf[2] = offset & 255;
	
	if (ddcci_write(mon, _buf, sizeof(_buf)) < 0) 
	{
		return -1;
	}
	
	return ddcci_read(mon, buf, len);
}

int ddcci_caps(struct monitor* mon)
{
	mon->caps.raw_caps = (char*)malloc(16);
	int bufferpos = 0;
	unsigned char buf[64];	/* 64 bytes chunk (was 35, but 173P+ send 43 bytes chunks) */
	int offset = 0;
	int len, i;
	int retries = 3;
	
	do {
		mon->caps.raw_caps[bufferpos] = 0;
		if (retries == 0) {
			return -1;
		}
		
		len = ddcci_raw_caps(mon, offset, buf, sizeof(buf));
		if (len < 0) {
			retries--;
			continue;
		}
		
		if (len < 3 || buf[0] != DDCCI_REPLY_CAPS || (buf[1] * 256 + buf[2]) != offset) 
		{
			if (!mon->probing || verbosity) {
				fprintf(stderr, _("Invalid sequence in caps.\n"));
			}
			retries--;
			continue;
		}

		mon->caps.raw_caps = (char*)realloc(mon->caps.raw_caps, bufferpos + len - 2);
		for (i = 3; i < len; i++) {
			mon->caps.raw_caps[bufferpos++] = buf[i];
		}
		
		offset += len - 3;
		
		retries = 3;
	} while (len != 3);

#if 0
	/* Test CAPS with binary data */
	mon->caps.raw_caps = realloc(mon->caps.raw_caps, 2048);
	strcpy(mon->caps.raw_caps, "( prot(monitor) type(crt) edid bin(128(");
	bufferpos = strlen(mon->caps.raw_caps);
	for (i = 0; i < 128; i++) {
		mon->caps.raw_caps[bufferpos++] = i;
	}
	strcpy(&mon->caps.raw_caps[bufferpos], ")) vdif bin(128(");
	bufferpos += strlen(")) vdif bin(128(");	
	for (i = 0; i < 128; i++) {
		mon->caps.raw_caps[bufferpos++] = i;
	}
	strcpy(&mon->caps.raw_caps[bufferpos], ")) vcp (10 12 16 18 1A 50 92)))");
	bufferpos += strlen(")) vcp (10 12 16 18 1A 50 92)))");
	/* End */
#endif
	
	mon->caps.raw_caps[bufferpos] = 0;

	char* last_substr = mon->caps.raw_caps;
	char* endptr;
	while ((last_substr = strstr(last_substr, "bin("))) {
		last_substr += 4;
		len = strtol(last_substr, &endptr, 0);
		if (*endptr != '(') {
			DSPRINT("Invalid bin in CAPS.\n");
			continue;
		}
		for (i = 0; i < len; i++) {
			*(++endptr) = '#';
		}
		last_substr += len;
	}
	
	ddcci_parse_caps(mon->caps.raw_caps, &mon->caps, 1);
	
	return bufferpos;
}

/* save current settings */
int ddcci_command(struct monitor* mon, unsigned char cmd)
{
	unsigned char _buf[1];

	_buf[0] = cmd;

	return ddcci_write(mon, _buf, sizeof(_buf));
}

int ddcci_read_edid(struct monitor* mon, int addr) 
{
	unsigned char buf[128];
	buf[0] = 0;	/* eeprom offset */
	
	if (i2c_write(mon, addr, buf, 1) > 0 &&
	    i2c_read(mon, addr, buf, sizeof(buf)) > 0) 
	{		
		if (buf[0] != 0 || buf[1] != 0xff || buf[2] != 0xff || buf[3] != 0xff ||
		    buf[4] != 0xff || buf[5] != 0xff || buf[6] != 0xff || buf[7] != 0)
		{
			if (!mon->probing || verbosity) {
				fprintf(stderr, _("Corrupted EDID at 0x%02x.\n"), addr);
			}
			return -1;
		}
		
		snprintf(mon->pnpid, 8, "%c%c%c%02X%02X", 
			((buf[8] >> 2) & 31) + 'A' - 1, 
			((buf[8] & 3) << 3) + (buf[9] >> 5) + 'A' - 1, 
			(buf[9] & 31) + 'A' - 1, buf[11], buf[10]);
		
		if (!mon->probing && verbosity) {
			int sn = buf[0xc] + (buf[0xd]<<8) + (buf[0xe]<<16) + (buf[0xf]<<24);
			DSPRINT(_("Serial number: %d\n"), sn);
			int week = buf[0x10];
			int year = buf[0x11] + 1990;
			DSPRINT(_("Manufactured: Week %d, %d\n"), week, year);
			int ver = buf[0x12];
			int rev = buf[0x13];
			DSPRINT(_("EDID version: %d.%d\n"), ver, rev);
			int maxwidth = buf[0x15];
			int maxheight = buf[0x16];
			DSPRINT(_("Maximum size: %d x %d (cm)\n"), maxwidth, maxheight);
			
			/* Parse more infos... */
		}

		if (strncmp(&buf[0x5f], "DASUNI", 6) == 0) {
			DSPRINT("found out DS monitor.\n");
			mon->DSMonitor = 1;
		}
		
		mon->digital = (buf[0x14] & 0x80);
		
		return 0;
	} 
	else {
		if (!mon->probing || verbosity) {
			fprintf(stderr, _("Reading EDID 0x%02x failed.\n"), addr);
		}
		return -1;
	}
}

/* Param probing indicates if we are probing for available devices (so we must be much less verbose)
   Returns :
  - 0 if OK
  - -1 if DDC/CI is not available
  - -2 if EDID is not available
  - -3 if file can't be opened 
*/
static int ddcci_open_with_addr(struct monitor* mon, const char* filename, int addr, int edid, int probing) 
{
	memset(mon, 0, sizeof(struct monitor));
	
	mon->probing = probing;
	
	if (strncmp(filename, "dev:", 4) == 0) {
		if ((mon->fd = open(filename+4, O_RDWR)) < 0) {
			if ((!probing) || verbosity)
				perror(filename);
			return -3;
		}
		mon->type = dev;
	}
#ifdef HAVE_DDCPCI
	else if (strncmp(filename, "pci:", 4) == 0) {
		if (verbosity)
			DSPRINT(_("Device: %s\n"), filename);
		
		struct query qopen;
		memset(&qopen, 0, sizeof(struct query));
		qopen.mtype = 1;
		qopen.qtype = QUERY_OPEN;
		
		sscanf(filename, "pci:%02x:%02x.%d-%d\n", 
			(unsigned int*)&qopen.bus.bus,
			(unsigned int*)&qopen.bus.dev,
			(unsigned int*)&qopen.bus.func,
			&qopen.bus.i2cbus);
		
		if (msgsnd(msqid, &qopen, QUERY_SIZE, IPC_NOWAIT) < 0) {
			perror(_("Error while sending open message"));
			return -3;
		}
		
		
		struct answer aopen;
		
		if (ddcpci_read(&aopen) < 0) {
			perror(_("Error while reading open message answer"));
			return -3;
		}
		
		mon->type = pci;
	}
#endif
	else if (strncmp(filename, "adl:", 4) == 0) {
		mon->adl_adapter = -1;
		mon->adl_display = -1;
		if (sscanf(filename, "adl:%d:%d", &mon->adl_adapter, &mon->adl_display) != 2){
			fprintf(stderr, _("Invalid filename (%s).\n"), filename);
			return -3;
		}

		if (amd_adl_check_display(mon->adl_adapter, mon->adl_display)){
			fprintf(stderr, _("ADL display not found (%s).\n"), filename);
			return -3;
		}

		mon->type = type_adl;
	}
	else {
		fprintf(stderr, _("Invalid filename (%s).\n"), filename);
		return -3;
	}
	
	mon->addr = addr;
	
	if (ddcci_read_edid(mon, edid) < 0) {
		return -2;
	}

	return 0;
	
	//ddcci_caps(mon);
	mon->fallback = 0; /* No fallback */
	
	{
		/* Fallback on manufacturer generic profile */
		char buffer[7];
		buffer[0] = 0;
		strncat(buffer, mon->pnpid, 3); /* copy manufacturer id */
		switch(mon->caps.type) {
		case lcd:
			strcat(buffer, "lcd");
			mon->fallback = 1;
			break;
		case crt:
			strcat(buffer, "crt");
			mon->fallback = 1;
			break;
		case unk:
			break;
		}
	}
	
	{
		if (ddcci_command(mon, DDCCI_COMMAND_PRESENCE) < 0) {
			return -1;
		}
	}
	
	return 0;
}

int ddcci_open(struct monitor* mon, const char* filename, int probing) 
{
	return ddcci_open_with_addr(mon, filename, DEFAULT_DDCCI_ADDR, DEFAULT_EDID_ADDR, probing);
}

int ddcci_save(struct monitor* mon) 
{
	return ddcci_command(mon, DDCCI_COMMAND_SAVE);
}

/* Returns :
  - 0 if OK
  - -1 if DDC/CI is not available
  - -3 if file can't be closed 
*/
int ddcci_close(struct monitor* mon)
{
	{ /* Alternate way of init mode detecting for unsupported monitors */
		if (strncmp(mon->pnpid, "SAM", 3) == 0) {
			if ((ddcci_writectrl(mon, DDCCI_CTRL, DDCCI_CTRL_DISABLE, 0)) < 0) {
				return -1;
			}
		}
	}
	
	int i;
	for (i = 0; i < 256; i++) {
		if(mon->caps.vcp[i]) {
			if (mon->caps.vcp[i]->values) {
				free(mon->caps.vcp[i]->values);
			}
			free(mon->caps.vcp[i]);
		}
	}
	
	if ((mon->fd > -1) && (close(mon->fd) < 0)) {
		return -3;
	}
	
	return 0;
}

struct monitor* ddcci_detect_DSMonitor(struct monitorlist* mlist)
{
	struct monitorlist* current = mlist;
	struct monitor* mon = NULL;
	int ret = -1;

	while (current != NULL)
	{
		if (current->DSMonitor == 1)
		{
			DSPRINT(_(" Found DSMonitor, device (%s).\n"),
				current->filename);
			mon = malloc(sizeof(struct monitor));
			ret = ddcci_open(mon, current->filename, 1);
			if (ret > -2) /* success get EDID */
			{
				if (mon->DSMonitor != 1)
					DSPRINT(_(" Open wrong monitor, device (%s).\n"),
						current->filename);
				break;
			}
			free(mon);
			mon = NULL;
			ddcci_close(mon);
		}
		current = current->next;
	}

	return mon;
}

void ddcci_probe_device(char* filename, struct monitorlist** current, struct monitorlist*** last) {
	struct monitor mon;
	int ret = ddcci_open(&mon, filename, 1);
	
	if (verbosity) {
		DSPRINT(_("ddcci_open returned %d\n"), ret);
	}
	
	if (ret > -2) { /* At least the EDID has been read correctly */
		(*current) = malloc(sizeof(struct monitorlist));
		(*current)->filename = filename;
		(*current)->supported = (ret == 0);
		{
			(*current)->name = malloc(32);
			snprintf((char*)(*current)->name, 32, _("Unknown monitor (%s)"), mon.pnpid);
		}
		(*current)->digital = mon.digital;
		(*current)->DSMonitor = mon.DSMonitor;
		(*current)->next = NULL;
		**last = (*current);
		*last = &(*current)->next;
	}
	else {
		free(filename);
	}

	ddcci_close(&mon);
}

struct monitorlist* ddcci_probe() {
	char* filename = NULL;
	
	struct monitorlist* list = NULL;
	struct monitorlist* current = NULL;
	struct monitorlist** last = &list;
	
	DSPRINT(_("Probing for available monitors"));
	if (verbosity)
		DSPRINT("...\n");
	fflush(stdout);
	
#ifdef HAVE_DDCPCI
	/* Fetch bus list from ddcpci */
	if (msqid >= 0) {
		struct query qlist;
		memset(&qlist, 0, sizeof(struct query));
		qlist.mtype = 1;
		qlist.qtype = QUERY_LIST;
		
		if (msgsnd(msqid, &qlist, QUERY_SIZE, IPC_NOWAIT) < 0) {
			perror(_("Error while sending list message"));
		}
		else {
			int len = 0, i;
			struct answer alist;
			char** filelist = NULL;
			
			while (1) {
				if (ddcpci_read(&alist) < 0){
					perror(_("Error while reading list entry"));
					break;
				}
				else {
					if (alist.last == 1) {
						break;
					}
					
					filelist = realloc(filelist, (len+1)*sizeof(struct answer));
					
					//printf("<==%02x:%02x.%d-%d\n", alist.bus.bus, alist.bus.dev, alist.bus.func, alist.bus.i2cbus);
					filelist[len] = malloc(32);
					
					snprintf(filelist[len], 32, "pci:%02x:%02x.%d-%d", 
						alist.bus.bus, alist.bus.dev, alist.bus.func, alist.bus.i2cbus);
					
					if (verbosity) {
						DSPRINT(_("Found PCI device (%s)\n"), filelist[len]);
					}
					
					len++;
				}
			}
			
			for (i = 0; i < len; i++) {
				ddcci_probe_device(filelist[i], &current, &last);
				if (!verbosity) {
					DSPRINT(".");
					fflush(stdout);
				}
			}
			free(filelist);
		}
	}
#endif
	
	/* Probe real I2C device */
	DIR *dirp;
	struct dirent *direntp;
	
	dirp = opendir("/dev/");
	
	while ((direntp = readdir(dirp)) != NULL)
	{
		if (!strncmp(direntp->d_name, "i2c-", 4))
		{
			filename = malloc(strlen(direntp->d_name)+12);
			
			snprintf(filename, strlen(direntp->d_name)+12, "dev:/dev/%s", direntp->d_name);
			
			if (verbosity) {
				DSPRINT(_("Found I2C device (%s)\n"), filename);
			}

			ddcci_probe_device(filename, &current, &last);
			if (!verbosity) {
				DSPRINT(".");
				fflush(stdout);
			}
		}
	}
	
	closedir(dirp);

	/* ADL probe */
	int adl_disp;

	for (adl_disp=0; adl_disp<amd_adl_get_displays_count(); adl_disp++){
		int adapter, display;
		if (amd_adl_get_display(adl_disp, &adapter, &display))
		    break;

			filename = malloc(64);
			snprintf(filename, 64, "adl:%d:%d", adapter, display);
			if (verbosity) {
				DSPRINT(_("Found ADL display (%s)\n"), filename);
			}
			ddcci_probe_device(filename, &current, &last);
			if (!verbosity) {
				DSPRINT(".");
				fflush(stdout);
		}
	}

	if (!verbosity)
		DSPRINT("\n");

	DSPRINT("exit.\n");	
	return list;
}

void ddcci_free_list(struct monitorlist* list) {
	if (list == NULL) {
		return;
	}
	free(list->filename);
	free(list->name);
	ddcci_free_list(list->next);
	free(list);
}

bool DSMonitorReadData(struct monitor *mon, unsigned char *data, unsigned char dataSize,
                       unsigned char *replayBuf, unsigned char replaySize)
{
	//unsigned char buf[12];
	int ret;
	ret = ddcci_read(mon, replayBuf, 8);
	if (ret == -1)
		return false;

	return true;
}

bool DSMonitorWriteData(struct monitor *mon, unsigned char *data, unsigned char dataSize, bool needACK)
{
	unsigned char buf[4];

	buf[0] = DDCCI_COMMAND_WRITE;
	buf[1] = 0x08;//VCPCODE
	buf[2] = data[0];
	buf[3] = data[1];

	int ret = ddcci_write(mon, buf, 4);

	return true;
}

bool send_screen_save_flag(struct monitor* mon, unsigned char flag)
{
    SignalData SD;
    unsigned char replay_data[128];
    unsigned char  replay_size = 8;
    
    memset((void *)&SD, 0, sizeof(SignalData));
    SD.SignalDataTypeHead.uSignalDataType = DS_PACKET_TYPE_SCREENSAVER;
    if (flag) {
        SD.SignalDataType.ScreenSaver.ucScreenSaver = 1;
    } else {
        SD.SignalDataType.ScreenSaver.ucScreenSaver = 0;
    }
    
    if (mon == 0)
        return false;

    //printf("----- issue screen %s command.\n", flag?"save":"unsave");
    
    DSMonitorWriteData(mon, (void *)&SD, sizeof(SignalData), false);
    
    usleep(600*1000);
    
    return DSMonitorReadData(mon, NULL, 0, replay_data, replay_size);

}

bool SendMonitorRes(struct monitor* mon, unsigned char ResMode)
{
    SignalData SD;
    unsigned char replay_data[128];
    unsigned char  replay_size = 8;
 
    memset((void *)&SD, 0, sizeof(SignalData));
    SD.SignalDataTypeHead.uSignalDataType = DS_PACKET_TYPE_MONITORRESOLUTION;
    SD.SignalDataType.MonitorResolution.ucMonitorResolution = ResMode;
    
    if (mon == 0)
        return false;
     
    DSMonitorWriteData(mon, (void *)&SD, sizeof(SignalData), false);
    
    usleep(600*1000);
    
    return DSMonitorReadData(mon, NULL, 0, replay_data, replay_size);
}


bool send_mode_selection(struct monitor* mon, unsigned char show_mode,
	unsigned char work_mode)
{
    SignalData SD;
    bool res;
    unsigned char replay_data[128];
    unsigned char replay_size = 11;
    unsigned int usleep_time = 1000;
    
    memset((void *)&SD, 0, sizeof(SignalData));
    SD.SignalDataTypeHead.uSignalDataType = DS_PACKET_TYPE_MODE;

    //printf("_____ %d %d ____\n", DS_PACKET_TYPE_MODE, show_mode);
    SD.SignalDataType.Mode.DisplayMode = show_mode;


    usleep_time = 800*1000;
    
    if (mon == 0)
        return false;
    
    DSMonitorWriteData(mon, (void *)&SD, sizeof(SignalData), false);
    
    usleep(usleep_time);
    
    res = DSMonitorReadData(mon, NULL, 0, replay_data, replay_size);
    
    return res;
}

#if 0
bool send_mode_selection(struct monitor* mon, unsigned char show_mode,
	unsigned char work_mode)
{
    SignalData SD;
    bool res;
    unsigned char replay_data[128];
    unsigned char replay_size = 8;
    unsigned int usleep_time = 1000;
    
    memset((void *)&SD, 0, sizeof(SignalData));
    SD.SignalDataTypeHead.uSignalDataType = DS_PACKET_TYPE_MODE;
    //SD.SignalDataType.Mode.InvalideMode = WorkMode;
    //SD.SignalDataType.Mode.DisplayMode1 = ShowMode;
    printf("_____ %d %d ____\n", work_mode, show_mode);
    
    switch (work_mode) {
        case DS_DISPLAY_NORMAL:
            SD.SignalDataType.Mode.DisplayMode1 = show_mode;
            break;
        case DS_DISPLAY_CENTRAL:
            SD.SignalDataType.Mode.DisplayMode2 = show_mode;
            break;
        case DS_DISPLAY_STRENTHEN_1:
            SD.SignalDataType.Mode.DisplayMode3 = show_mode;
            break;
        case DS_DISPLAY_STRENTHEN_2:
            SD.SignalDataType.Mode.DisplayMode4 = show_mode;
            break;
        default:
            break;
    }
    
    switch (show_mode) {
        case DS_MODE_A2:
            usleep_time = 800*1000;
            break;
        case DS_MODE_A16:
            usleep_time = 800*1000;
            break;
            
    }
    
    if (mon == NULL)
        return false;
    
    DSMonitorWriteData(mon, (void *)&SD, sizeof(SignalData), false);
    
    usleep(usleep_time);
    
    res = DSMonitorReadData(mon, NULL, 0, replay_data, replay_size);

    for (int i = 0; i < replay_size; i++)
	printf("%.02x ", replay_data[i]);

	printf("\n");

}
#endif

bool send_monitor_threshold(struct monitor* mon, unsigned short threshold)
{
    SignalData SD;
    unsigned char replay_data[128];
    unsigned char replay_size = 8;
    
    memset((void *)&SD, 0, sizeof(SignalData));
    SD.SignalDataTypeHead.uSignalDataType = DS_PACKET_TYPE_THRESHOLD;
    SD.SignalDataType.Threshold.uThreshold = threshold;
    
    if (mon == 0)
        return false;
    
    DSMonitorWriteData(mon, (void *)&SD, sizeof(SignalData), false);

    //printf(" --- send monitor threshold %d.\n", threshold);
    
    usleep(500*1000);
    
    return DSMonitorReadData(mon, NULL, 0, replay_data, replay_size);
}

bool SendMonitorFloydThreshold(struct monitor* mon, unsigned short Threshold)
{
    SignalData SD;
    unsigned char replay_data[128];
    unsigned char replay_size = 11;
    
    memset((void *)&SD, 0, sizeof(SignalData));
    SD.SignalDataTypeHead.uSignalDataType = DS_PACKET_TYPE_THRESHOLD_FLOYD;
    SD.SignalDataType.Threshold.uThreshold = Threshold;
    
    if (mon == 0)
        return false;
    
    DSMonitorWriteData(mon, (void *)&SD, sizeof(SignalData), false);
    
    usleep(500*1000);
    
    return DSMonitorReadData(mon, NULL, 0, replay_data, replay_size);
}

bool SendMonitorA2Threshold(struct monitor* mon, unsigned short Threshold)
{
    SignalData SD;
    unsigned char replay_data[128];
    unsigned char replay_size = 11;
    
    memset((void *)&SD, 0, sizeof(SignalData));
    SD.SignalDataTypeHead.uSignalDataType = DS_PACKET_TYPE_THRESHOLD_A2;
    SD.SignalDataType.Threshold.uThreshold = Threshold;
    
    if (mon == 0)
        return false;
    
    DSMonitorWriteData(mon, (void *)&SD, sizeof(SignalData), false);
    
    usleep(500*1000);
    
    return DSMonitorReadData(mon, NULL, 0, replay_data, replay_size);
}

bool SendMonitorLowA2Threshold(struct monitor* mon, unsigned short Threshold)
{
    SignalData SD;
    unsigned char replay_data[128];
    unsigned char replay_size = 11;
    
    memset((void *)&SD, 0, sizeof(SignalData));
    SD.SignalDataTypeHead.uSignalDataType = DS_PACKET_TYPE_THRESHOLD_LOW_A2;
    SD.SignalDataType.Threshold.uThreshold = Threshold;
    
    if (mon == 0)
        return false;
    
    DSMonitorWriteData(mon, (void *)&SD, sizeof(SignalData), false);
    
    usleep(500*1000);
    
    return DSMonitorReadData(mon, NULL, 0, replay_data, replay_size);
}

bool SendMonitorLowA5Threshold(struct monitor* mon, unsigned short Threshold)
{
    SignalData SD;
    unsigned char replay_data[128];
    unsigned char replay_size = 11;
    
    memset((void *)&SD, 0, sizeof(SignalData));
    SD.SignalDataTypeHead.uSignalDataType = DS_PACKET_TYPE_THRESHOLD_LOW_A5;
    SD.SignalDataType.Threshold.uThreshold = Threshold;
    
    if (mon == 0)
        return false;
    
    DSMonitorWriteData(mon, (void *)&SD, sizeof(SignalData), false);
    
    usleep(500*1000);
    
    return DSMonitorReadData(mon, NULL, 0, replay_data, replay_size);
}

bool send_clear_monitor(struct monitor* mon, unsigned short soft_hard)
{
    SignalData SD;
    unsigned char replay_data[128];
    unsigned char replay_size = 8;
    
    memset((void *)&SD, 0, sizeof(SignalData));
    SD.SignalDataTypeHead.uSignalDataType = DS_PACKET_TYPE_CLEANMONITOR;
    if (soft_hard) {
        SD.SignalDataType.CleanMonitor.uCleanMonitorSoft = 1;
    } else {
        SD.SignalDataType.CleanMonitor.uCleanMonitorHard = 1;
    }
    
    if (mon == 0)
        return false;
    
    //printf("call clean blur++++++++++++\n");
    
    DSMonitorWriteData(mon, (void *)&SD, sizeof(SignalData), false);
    
    usleep(500*1000);
    
    return DSMonitorReadData(mon, NULL, 0, replay_data, replay_size);
}

bool send_heart_signal(struct monitor* mon, bool heart)
{
    unsigned char replay_data[11];
    unsigned char replay_size = 8;
    SignalData SD;
    memset((void *)&SD, 0, sizeof(SignalData));
    SD.SignalDataTypeHead.uSignalDataType = DS_PACKET_TYPE_BEAT;
    if (heart) {
        SD.SignalDataType.Beat.uBeatSign = 1;
        SD.SignalDataType.Beat.uBeatSignOff = 0;
    } else {
        SD.SignalDataType.Beat.uBeatSign = 0;
        SD.SignalDataType.Beat.uBeatSignOff = 1;
    }
    
    if (mon == 0)
        return false;
    
    return DSMonitorWriteData(mon, (void *)&SD, sizeof(SignalData), false);
/*
    usleep(200);

    DSMonitorReadData(mon, NULL, 0, replay_data, replay_size);

    for (int i = 0; i < replay_size; i++)
	printf("%.02x ", replay_data[i]);

	printf("\n");

    return true;
*/
}
