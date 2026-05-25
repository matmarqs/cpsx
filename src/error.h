#ifndef _ERROR_H
#define _ERROR_H

#include <stdarg.h>

#define	MAXLINE	4096			/* max line length */

void err_quit(const char *fmt, ...);
void err_debug(const char *fmt, ...);

#endif // _ERROR_H
