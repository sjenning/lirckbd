all:
	gcc -o lirckb lirckb.c logging.c parse.c uinpdev.c -llirc_client

clean:
	rm lirckb
