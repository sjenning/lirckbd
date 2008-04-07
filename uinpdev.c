#include <stdio.h>
#include <stdlib.h>
#include "lirc/lirc_client.h"
#include <linux/input.h>
#include <linux/uinput.h>
#include <sys/time.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "logging.h"
#include "lirckb.h"
#include "uinpdev.h"

int
uinpdev_init(lirckb_cmd_t *cmdlist)
{
	struct uinput_user_dev uinp;
	lirckb_cmd_t *mycmd = cmdlist;
	int i, uinpdev;
	
	/* open the uinput device provided by the uinput kernel ext */
	uinpdev = open("/dev/uinput", O_WRONLY | O_NDELAY);
	if(uinpdev==-1)
	{
		logerr("couldn't open uinput device");
		return -1;
	}
	
	/* initialize our device configuration structure */
	memset(&uinp,0,sizeof(uinp));

	/* set device configuration */
	strncpy(uinp.name,LIRCKB_UINPUT_DEVNAME,UINPUT_MAX_NAME_SIZE);
	uinp.id.version = 1;
	uinp.id.bustype = BUS_USB;

	/* define a keyboard device */
	ioctl(uinpdev, UI_SET_EVBIT, EV_KEY);

	/* for each key used in the lirckb configuration, enable
	 * that key on our uinput device */
	while(mycmd)
	{
		for(i=0; i<mycmd->numkbcodes; i++)
		{
			ioctl(uinpdev, UI_SET_KEYBIT, mycmd->kbcodes[i]);
		}
		mycmd = mycmd->next;
	}

	/* write the device configuration to the device */
	write(uinpdev,&uinp,sizeof(uinp));

	/* create the uinput device */
	if(ioctl(uinpdev, UI_DEV_CREATE))
	{
		logerr("failed to create uinput device");
		return -1;
	}
	
	return uinpdev;
}

void
uinpdev_handle_cmd(int uinpdev, lirckb_cmd_t *cmd)
{
	struct input_event event;
	int i;
	
#ifdef DEBUG
	fprintf(stderr,"DEBUG: %s command received\n",cmd->command);
#endif
	
	
	/* "press button" */
	memset(&event,0,sizeof(event));
	gettimeofday(&event.time,NULL);
	event.type = EV_KEY;
	event.value = 1;
	
	for(i=0; i<cmd->numkbcodes; i++)
	{
		event.code = cmd->kbcodes[i];
		write(uinpdev,&event,sizeof(event));
	}
	
	event.type = EV_SYN;
	event.code = SYN_REPORT;
	event.value = 0;
	write(uinpdev,&event,sizeof(event));

	/* "release button" */
	memset(&event,0,sizeof(event));
	gettimeofday(&event.time,NULL);
	event.type = EV_KEY;
	event.value = 0;
	
	for(i=0; i<cmd->numkbcodes; i++)
	{
		event.code = cmd->kbcodes[i];
		write(uinpdev,&event,sizeof(event));
	}
	
	event.type = EV_SYN;
	event.code = SYN_REPORT;
	event.value = 0;
	write(uinpdev,&event,sizeof(event));
}

void
uinpdev_cleanup(int uinpdev)
{
	/* destroy the input device */
	ioctl(uinpdev, UI_DEV_DESTROY);
	
	/* close the uinput device */
	close(uinpdev);
}
