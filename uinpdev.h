#ifndef _UINPDEV_H_
#define _UINPDEV_H_

#define LIRCKB_UINPUT_DEVNAME "LIRC Keyboard"

extern
int
uinpdev_init(lirckbd_cmd_t *cmdlist);

extern
void
uinpdev_handle_cmd(int uinpdev, lirckbd_cmd_t *cmd);

extern
void
uinpdev_cleanup(int uinpdev);

#endif /* _UINPDEV_H_ */
