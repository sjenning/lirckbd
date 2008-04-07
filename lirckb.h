#ifndef _LIRCKB_H_
#define _LIRCKB_H_

#define LIRCKB_CMD_SIZE 32
#define LIRCKB_CMD_CODES 4
typedef struct lirckb_cmd
{
	struct lirckb_cmd *next;
	char *command;
	int numkbcodes;
	int kbcodes[LIRCKB_CMD_CODES];
} lirckb_cmd_t;

extern lirckb_cmd_t *cmd_map;

extern 
lirckb_cmd_t* 
lirckb_get_cmd(lirckb_cmd_t *cmdlist, const char *command);

extern
void
lirckb_cleanup(int signum);

#endif /* _LIRCKB_H_ */

