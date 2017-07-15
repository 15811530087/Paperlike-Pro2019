#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <stdarg.h>
#include <signal.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <string.h>
#include <linux/kd.h>
#include <linux/keyboard.h>
#ifndef SIMULATION
#include "thread.h"
#endif

struct termios old;
const char *progname = "Keyboard event";
static char *conspath[] = {
	"/proc/self/fd/0",
	"/dev/tty",
	"/dev/tty0",
	"/dev/vc/0",
	"/dev/systty",
	"/dev/console",
	NULL
};

void kbd_warning(const int errnum, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);

	fprintf(stderr, "%s: ", progname);
	vfprintf(stderr, fmt, ap);

	if (errnum > 0)
		fprintf(stderr, ": %s\n", strerror(errnum));

	va_end(ap);
	return;
}

void kbd_error(const int exitnum, const int errnum, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	fprintf(stderr, "%s: ", progname);
	vfprintf(stderr, fmt, ap);

	va_end(ap);

	if (errnum > 0)
		fprintf(stderr, ": %s\n", strerror(errnum));

	exit(exitnum);
}

static int
is_a_console(int fd) {
	char arg;

	arg = 0;
	return (isatty (fd)
		&& ioctl(fd, KDGKBTYPE, &arg) == 0
		&& ((arg == KB_101) || (arg == KB_84)));
}

static int
open_a_console(const char *fnam) {
	int fd;

	/*
	 * For ioctl purposes we only need some fd and permissions
	 * do not matter. But setfont:activatemap() does a write.
	 */
	fd = open(fnam, O_RDWR);
	if (fd < 0)
		fd = open(fnam, O_WRONLY);
	if (fd < 0)
		fd = open(fnam, O_RDONLY);
	if (fd < 0)
		return -1;
	return fd;
}

int
getfd(const char *fnam) {
	int fd, i;

	if (fnam) {
		if ((fd = open_a_console(fnam)) >= 0) {
			if (is_a_console(fd))
				return fd;
			close(fd);
		}
		fprintf(stderr, "Couldn't open %s\n", fnam);
		exit(1);
	}

	for (i = 0; conspath[i]; i++) {
		if ((fd = open_a_console(conspath[i])) >= 0) {
			DSPRINT("console is %s \n", conspath[i]);
			if (is_a_console(fd))
				return fd;
			close(fd);
		}
	}

	for (fd = 0; fd < 3; fd++)
		if (is_a_console(fd))
			return fd;

	fprintf(stderr,
		"Couldn't get a file descriptor referring to the console\n");

	/* total failure */
	exit(1);
}

#ifdef SIMULATION
int main(int argc, char **argv)
#else
void *detect_keyboard_event(void *param)
#endif
{
	int c;
	int fd;
	int show_keycodes = 1;
	int print_ascii = 0;
	unsigned int pre_key_code = -1;

	struct termios new;
	unsigned char buf[18];	/* divisible by 3 */
	int i, n;

	fd = getfd(NULL);

	if (tcgetattr(fd, &old) == -1)
		kbd_warning(errno, "tcgetattr");
	if (tcgetattr(fd, &new) == -1)
		kbd_warning(errno, "tcgetattr");

	new.c_lflag &= ~ (ICANON | ECHO | ISIG);
	new.c_iflag = 0;
	new.c_cc[VMIN] = sizeof(buf);
	new.c_cc[VTIME] = 1;	/* 0.1 sec intercharacter timeout */

	if (tcsetattr(fd, TCSAFLUSH, &new) == -1)
		kbd_warning(errno, "tcsetattr");
	if (ioctl(fd, KDSKBMODE, show_keycodes ? K_MEDIUMRAW : K_RAW)) {
		kbd_error(EXIT_FAILURE, errno, "ioctl KDSKBMODE");
	}

	if (tcsetattr(fd, TCSAFLUSH, &new) == -1)
		kbd_warning(errno, "tcsetattr");
	if (ioctl(fd, KDSKBMODE, show_keycodes ? K_MEDIUMRAW : K_RAW)) {
		kbd_error(EXIT_FAILURE, errno, "ioctl KDSKBMODE");
	}

	while (1) {
		//alarm(10);
		n = read(fd, buf, sizeof(buf));
		i = 0;
		while (i < n) {
			int kc;
			int j = i;
			char *s;

			s = (buf[i] & 0x80) ? "release" : "press";

			if (i+2 < n && (buf[i] & 0x7f) == 0
				&& (buf[i+1] & 0x80) != 0
				&& (buf[i+2] & 0x80) != 0) {
				kc = ((buf[i+1] & 0x7f) << 7) |
					(buf[i+2] & 0x7f);
				i += 3;
			} else {
				kc = (buf[i] & 0x7f);
				i++;
			}

			if (buf[j] & 0x80)
				continue;

			#ifndef SIMULATION
			screen_save_count = 0;
			#endif

			DSPRINT("keycode %3d %s\n", kc, s);
			#ifndef SIMULATION
			//ALT(56) C(46) H(35)
			if ((soft_hand_refresh_enable != 0) &&
					((pre_key_code == 56) && (kc == 46))) {
				DSPRINT("pressed soft refresh.\n");
				clear_blur_flag = 1;
			}
			if ((hard_hand_refresh_enable != 0) &&
					((pre_key_code == 56) && (kc == 35))) {
				DSPRINT("pressed hard refresh.\n");
				clear_blur_flag = 2;
			}
			#endif
			pre_key_code = kc;
		}
	}
#ifdef SIMULATION
	return 0;
#endif
}
