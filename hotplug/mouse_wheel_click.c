#include <stdio.h>
#include <string.h>
//#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef SIMULATION
#define DSPRINT printf
int main(void)
#else
#include "thread.h"
void *mouse_wheel_click(void *param)
#endif
{

    int mouse_fd = open("/dev/input/mice", O_RDONLY | O_NONBLOCK);
    signed char input[4];
    ssize_t rd_cnt;

    if(mouse_fd < 0)
    {
        perror("Could not open /dev/input/mice");
        return 0;
    }

    while(1)
    {
        errno = 0;
        rd_cnt = read(mouse_fd, input, 4);
        if(rd_cnt <= 0 && errno != EAGAIN)
        {
            DSPRINT("Mouse read error:\n");
        }
        else
        {
	#ifdef SIMULATION
            for(int i = 0; i < rd_cnt; i++)
            {
                DSPRINT("%d", input[i]);
                if(i == rd_cnt - 1)
                {
                    DSPRINT("\n");
                }
                else
                {
                    DSPRINT("\t");
                }
            }
	#else
		screen_save_count = 0;
	#endif
        }
    }

    return 0;
}
