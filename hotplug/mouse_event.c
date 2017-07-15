#include <stdio.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <stdlib.h>

#ifdef SIMULATION
int main(int argc, char **argv)
#else
#include "thread.h"
void *mouse_event(void *param)
#endif
{
	Window root, child;
	int rootX, rootY, winX, winY;
	int pre_rootX = -1, pre_rootY = -1;
	unsigned int mask;

	Display *display;
	XEvent xevent;
	Window window;
	if((display = XOpenDisplay(NULL)) == NULL )
		#ifdef SIMULATION
		return 0;
		#else
		return NULL;
		#endif
	window = DefaultRootWindow(display);

	while(1) {
		XQueryPointer(display, window, &root, &child,
			&rootX, &rootY, &winX, &winY, &mask);
		DSPRINT("%d %d.\n", rootX, rootY);
		#ifndef SIMULATION
		if ((rootX != pre_rootX) && (rootY != pre_rootY)) {
			pre_rootX = rootX;
			pre_rootY = rootY;
			screen_save_count = 0;
		}
		#endif
			
		sleep(1);
	}
	#ifdef SIMULATION
	return 0;
	#else
	return NULL;
	#endif
}

void restore_default_screen_resolution()
{
	system("ResChange 1600x1200");
	//system("xrandr --output DP-1-1 --mode 1600x1200");
}

void set_default_screen_resolution()
{
	system("ResChange 800x600");
	//system("xrandr --output DP-1-1 --mode 800x600");
}
