#include <stdio.h>
#include "logging.h"
#include "lirckbd.h"

void 
logdbg(const char *str)
{
	if(debug)
		fprintf(stderr,"DEBUG: %s\n",str);
}

void
logerr(const char *str)
{
	fprintf(stderr,"ERROR: %s\n",str);
}

