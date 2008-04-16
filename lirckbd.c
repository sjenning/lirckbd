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
#include <ctype.h>

#include "lirckbd.h"
#include "logging.h"
#include "parse.h"
#include "uinpdev.h"

/* globals now */
lirckbd_cmd_t *cmdlist = NULL;
struct lirc_config *config = NULL;
int uinpdev = 0, debug = 0;
char *configfile = "/etc/lirckbd.conf";

int
main(int argc, char *argv[])
{	
	char *code, *c, opt;
	int rc;
	lirckbd_cmd_t *cmd;

	while((opt = getopt(argc,argv,"d"))!=-1)
	{
		switch(opt)
		{
			case 'c':
				configfile = optarg;
				logdbg("config file set to %s");
			break;
			case 'd':
				debug = 1;
				logdbg("debug output enabled");
			break;
			case '?':
				if(optopt=='c')
					fprintf(stderr,"option -c requires argument\n");
				else if(isprint(optopt))
					fprintf(stderr,"unknown option '-%c'\n",optopt);
				else
					fprintf(stderr,"unknown option\n");
			/* fall-through case on purpose */
			default:
				printf("usage:\n");
				printf("%s: [-c configfile] [-d]\n",argv[0]);
				exit(1);
		}
	}

	/* read lirckbd configuration */
	logdbg("read lirckbd configuration");
	if((cmdlist = read_lirckbd_config(configfile))==NULL)
	{
		logerr("failed to read lirckbd configuration");
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
	if(lirc_init("lirckbd",1)==-1)
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
	//if(fork())
	//	exit(EXIT_SUCCESS);

	/* register SIGTERM handler */
	signal(SIGTERM, lirckbd_cleanup);

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
			cmd = lirckbd_get_cmd(cmdlist,c);
			
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


lirckbd_cmd_t * 
lirckbd_get_cmd(lirckbd_cmd_t *cmdlist, const char *command)
{
	lirckbd_cmd_t *mycmd = cmdlist;
	while(mycmd)
	{
		if(strncmp(command,mycmd->command,LIRCKB_CMD_SIZE)==0)
			break;
		mycmd = mycmd->next;
	}
	return mycmd;
}

void
lirckbd_cleanup(int signum)
{
	logdbg("SIGTERM received");

	if(cmdlist) free_lirckbd_config(cmdlist);
	
	if(config) lirc_freeconfig(config);
	lirc_deinit();
	
	if(uinpdev) uinpdev_cleanup(uinpdev);
}

