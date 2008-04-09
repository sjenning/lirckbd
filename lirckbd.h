#ifndef _LIRCKB_H_
#define _LIRCKB_H_

#define LIRCKB_CMD_SIZE 32
#define LIRCKB_CMD_CODES 4
typedef struct lirckbd_cmd
{
	struct lirckbd_cmd *next;
	char *command;
	int numkbcodes;
	int kbcodes[LIRCKB_CMD_CODES];
} lirckbd_cmd_t;

extern lirckbd_cmd_t *cmd_map;
extern int debug;

extern 
lirckbd_cmd_t* 
lirckbd_get_cmd(lirckbd_cmd_t *cmdlist, const char *command);

extern
void
lirckbd_cleanup(int signum);

#endif /* _LIRCKB_H_ */

