#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lirckbd.h"
#include "parse.h"
#include "logging.h"

/* internal prototype */
int
get_keycode(const char *keyname);

keymap_t lirckbd_keymap[LIRCKB_KEYMAP_SIZE] =
{	{"ESC", KEY_ESC },
	{ "1",	KEY_1 },
	{ "2",	KEY_2 },
	{ "3",	KEY_3 },
	{ "4",	KEY_4 },
	{ "5",	KEY_5 },
	{ "6",	KEY_6 },
	{ "7",	KEY_7 },
	{ "8",	KEY_8 },
	{ "9",	KEY_9 },
	{ "0",	KEY_0 },
	{ "MINUS",	KEY_MINUS },
	{ "EQUAL",	KEY_EQUAL },
	{ "BACKSPACE",	KEY_BACKSPACE },
	{ "TAB",	KEY_TAB },
	{ "Q",	KEY_Q },
	{ "W",	KEY_W },
	{ "E",	KEY_E },
	{ "R",	KEY_R },
	{ "T",	KEY_T },
	{ "Y",	KEY_Y },
	{ "U",	KEY_U },
	{ "I",	KEY_I },
	{ "O",	KEY_O },
	{ "P",	KEY_P },
	{ "LEFTBRACE",	KEY_LEFTBRACE },
	{ "RIGHTBRACE",	KEY_RIGHTBRACE },
	{ "ENTER",	KEY_ENTER },
	{ "LEFTCTRL",	KEY_LEFTCTRL },
	{ "A",	KEY_A },
	{ "S",	KEY_S },
	{ "D",	KEY_D },
	{ "F",	KEY_F },
	{ "G",	KEY_G },
	{ "H",	KEY_H },
	{ "J",	KEY_J },
	{ "K",	KEY_K },
	{ "L",	KEY_L },
	{ "SEMICOLON",	KEY_SEMICOLON },
	{ "APOSTROPHE",	KEY_APOSTROPHE },
	{ "LEFTSHIFT",	KEY_LEFTSHIFT },
	{ "BACKSLASH",	KEY_BACKSLASH },
	{ "Z",	KEY_Z },
	{ "X",	KEY_X },
	{ "C",	KEY_C },
	{ "V",	KEY_V },
	{ "B",	KEY_B },
	{ "N",	KEY_N },
	{ "M",	KEY_M },
	{ "COMMA",	KEY_COMMA },
	{ "DOT",	KEY_DOT },
	{ "SLASH",	KEY_SLASH },
	{ "RIGHTSHIFT",	KEY_RIGHTSHIFT },
	{ "LEFTALT",	KEY_LEFTALT },
	{ "SPACE",	KEY_SPACE },
	{ "CAPSLOCK",	KEY_CAPSLOCK },
	{ "F1",	KEY_F1 },
	{ "F2",	KEY_F2 },
	{ "F3",	KEY_F3 },
	{ "F4",	KEY_F4 },
	{ "F5",	KEY_F5 },
	{ "F6",	KEY_F6 },
	{ "F7",	KEY_F7 },
	{ "F8",	KEY_F8 },
	{ "F9",	KEY_F9 },
	{ "F10",	KEY_F10 },
	{ "F11",	KEY_F11 },
	{ "F12",	KEY_F12 },
	{ "RIGHTCTRL",	KEY_RIGHTCTRL },
	{ "RIGHTALT",	KEY_RIGHTALT },
	{ "HOME",	KEY_HOME },
	{ "UP",	KEY_UP },
	{ "PAGEUP",	KEY_PAGEUP },
	{ "LEFT",	KEY_LEFT },
	{ "RIGHT",	KEY_RIGHT },
	{ "END",	KEY_END },
	{ "DOWN",	KEY_DOWN },
	{ "PAGEDOWN",	KEY_PAGEDOWN },
	{ "INSERT",	KEY_INSERT },
	{ "DELETE",	KEY_DELETE }
};

lirckbd_cmd_t* 
read_lirckbd_config(const char *filename)
{
	FILE *cfgfile = NULL;
	char linebuf[LINEBUF_SIZE];
	char *ptr = NULL;
	unsigned int i, linenum = 0;
	lirckbd_cmd_t *cmdnode = NULL, *tmpnode = NULL;

	logdbg("opening config file");
	if((cfgfile = fopen(filename,"r")) == NULL)
	{
		logerr("failed to open lirckbd configuration file");
		return NULL;
	}

	logdbg("scanning config file");
	while(fgets(linebuf,LINEBUF_SIZE,cfgfile))
	{
		/* increment the line number counter */
		linenum++;

		/* replace newline with terminator */
		if((ptr=strchr(linebuf,'\n')))
			*ptr='\0';

		/* chop off comments */
		if((ptr=strchr(linebuf,'#')))
			*ptr='\0';

		/* check for emtpy lines */
		if(linebuf[0]=='\0')
			continue;

		/* allocate new lirckbd_cmd node */
		tmpnode = cmdnode;
		cmdnode = (lirckbd_cmd_t*)malloc(sizeof(lirckbd_cmd_t));
		if(cmdnode==NULL)
		{
			logerr("out of memory");
			/* tmpnode is the head of the part of the list that 
			 * was successfully allocated */
			free_lirckbd_config(tmpnode);
			return NULL;
		}
		memset(cmdnode,0,sizeof(cmdnode));

		
		/* link cmdnode to front of LL */
		cmdnode->next = tmpnode;

		/* get the command */
		ptr=strtok(linebuf," ");
		if(!ptr)
		{
			fprintf(stderr,"no command at line %u\n",linenum);
			free_lirckbd_config(cmdnode);
			return NULL;
		}

		/* copy the command token into the cmdnode */
		cmdnode->command = strdup(ptr);

		/* make sure command is followed by equals */
		ptr=strtok(NULL," ");
		
		if(*ptr!='=')
		{
			fprintf(stderr,"no '=' found at line %u\n",linenum);
			free_lirckbd_config(cmdnode);
			return NULL;
		}

		/* get the list of keystrokes */
		for(i=0, ptr=strtok(NULL," "); (ptr && i<4); i++, ptr=strtok(NULL," "))
		{
			if((cmdnode->kbcodes[i] = get_keycode(ptr))==-1)
			{
				fprintf(stderr,"unknown key '%s' at line %u\n",ptr,linenum);
				free_lirckbd_config(cmdnode);
				return NULL;
			}
		}
		
		/* if command has no keystrokes, error */
		if(i==0)
		{
			fprintf(stderr,"command has no keystrokes at line %u\n",linenum);
			free_lirckbd_config(cmdnode);
			return NULL;
		}
		cmdnode->numkbcodes = i;
	}

	return cmdnode;
}

void 
free_lirckbd_config(lirckbd_cmd_t *cmdlist)
{
	lirckbd_cmd_t *cmd, *nextcmd;
	cmd = cmdlist;
	while(cmd)
	{
		nextcmd = cmd->next;
		if(cmd->command) 
			free(cmd->command);
		free(cmd);
		cmd = nextcmd;
	}
}

int
get_keycode(const char *keyname)
{
	int i;
	
	for(i=0; i<LIRCKB_KEYMAP_SIZE; i++)
		if(strncmp(keyname,lirckbd_keymap[i].keyname,LIRCKB_KEYNAME_SIZE)==0)
			return lirckbd_keymap[i].keycode;
	
	return -1;
}

