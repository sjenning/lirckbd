#ifndef _PARSE_H_
#define _PARSE_H_

#include <linux/input.h>

#define LINEBUF_SIZE 256

#define LIRCKB_KEYNAME_SIZE 32
typedef struct keymap
{
	char *keyname;
	int keycode;
} keymap_t;

#define LIRCKB_KEYMAP_SIZE 80
extern keymap_t lirckb_keymap[LIRCKB_KEYMAP_SIZE];

extern
lirckb_cmd_t* 
read_lirckb_config(const char *filename);

extern
void
free_lirckb_config(lirckb_cmd_t *cmdlist);

#endif /* _PARSE_H_ */
