#include "error.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

static void err_doit(int errnoflag, int error, const char *fmt, va_list ap)
{
	char	buf[MAXLINE];

	vsnprintf(buf, MAXLINE-1, fmt, ap);
	if (errnoflag)
		snprintf(buf+strlen(buf), MAXLINE-strlen(buf)-1, ": %s",
		  strerror(error));
	strcat(buf, "\n");
	fflush(stdout);		/* in case stdout and stderr are the same */
	fputs(buf, stderr);
	fflush(NULL);		/* flushes all stdio output streams */
}

void err_quit(const char *fmt, ...)
{
	va_list		ap;

	va_start(ap, fmt);
	err_doit(0, 0, fmt, ap);
	va_end(ap);
	exit(1);
}

// it's better to trigger the debugger than to quit
void err_debug(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	err_doit(0, 0, fmt, ap);
	va_end(ap);

    __asm__("int3"); // trigger the debugger
}
