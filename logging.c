#include <stdio.h>
#include "logging.h"

void 
logdbg(const char *str)
{
#ifdef DEBUG
	fprintf(stderr,"DEBUG: %s\n",str);
#endif
}

void
logerr(const char *str)
{
	fprintf(stderr,"ERROR: %s\n",str);
}

