/* version 1.0 */
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lirc/lirc_client.h"
#include <linux/input.h>
#include <linux/uinput.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <signal.h>

#include "lirckb.h"
#include "logging.h"
#include "parse.h"

/* globals now */
lirckb_cmd_t *cmdlist = NULL;
struct lirc_config *config = NULL;
int uinpdev = 0;
int daemonize = 0;

int
main(int argc, char *argv[])
{	
	char *code, *c, opt;
	int rc;
	lirckb_cmd_t *cmd;
	int i;
	
	while((opt=getopt(argc,argv,"d"))!=-1)
	{
		switch(opt)
		{
		case 'd':
			logdbg("daemonize set");
			daemonize = 1;
			break;
		}
	}

	/* read lirckb configuration */
	logdbg("read lirckb configuration");
	if((cmdlist=read_lirckb_config("/etc/lirckb.conf"))==NULL)
	{
		logerr("failed to read lirckb configuration");
		return EXIT_FAILURE;
	}
	
	logdbg("initialize uinput device");
	/* initialize uinput device */
	if((uinpdev = uinpdev_init(cmdlist))==-1)
	{
		logerr("failed to init uinpdev");
		return EXIT_FAILURE;	
	}

	logdbg("initialize lirc");
	/* initialize lirc */
	if(lirc_init("lirckb",1)==-1)
	{
		logerr("lirc_init failed");
		return(EXIT_FAILURE);
	}

	logdbg("read default lirc config file");
	/* read default lirc config file */
	if(lirc_readconfig(NULL,&config,NULL)==-1) 
	{
		logerr("lirc_readconfig failed");
		return(EXIT_FAILURE);
	}
	
	/* fork the child process (the daemon) and exit the parent */
	if(daemonize && fork())
		exit(EXIT_SUCCESS);
	
	/* register SIGTERM handler */
	signal(SIGTERM, lirckb_cleanup);

	logdbg("wait for the next lirc input");
	/* wait for the next lirc input */
	while(lirc_nextcode(&code)==0)
	{
		/* if the code is NULL, discard */
		if(!code) continue;

		/* for every config command configured */
		while((rc = lirc_code2char(config,code,&c))==0 && c)
		{
			/* lookup the command in the cmdlist */
			cmd = lirckb_get_cmd(cmdlist,c);
			
			/* if no command was found ignore */
			if(!cmd) continue;
			
			/* handle the command */
			uinpdev_handle_cmd(uinpdev,cmd);
		}

		/* cleanup */
		free(code);

		/* there was a fatal problem in lirc_code2char */
		if(rc==-1) break;
	}

	/* done */
	return(EXIT_SUCCESS);
}


lirckb_cmd_t * 
lirckb_get_cmd(lirckb_cmd_t *cmdlist, const char *command)
{
	lirckb_cmd_t *mycmd = cmdlist;
	while(mycmd)
	{
		if(strncmp(command,mycmd->command,LIRCKB_CMD_SIZE)==0)
			break;
		mycmd = mycmd->next;
	}
	return mycmd;
}

void
lirckb_cleanup(int signum)
{
	logdbg("SIGTERM received");

	if(cmdlist) free_lirckb_config(cmdlist);
	
	if(config) lirc_freeconfig(config);
	lirc_deinit();
	
	if(uinpdev) uinpdev_cleanup(uinpdev);

}
