/*	$NetBSD: __strsignal.c,v 1.13 1997/07/13 20:24:10 christos Exp $	*/

/*
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#if defined(LIBC_SCCS) && !defined(lint)
#if 0
static char *sccsid = "@(#)strerror.c	5.6 (Berkeley) 5/4/91";
#else
__RCSID("$NetBSD: __strsignal.c,v 1.13 1997/07/13 20:24:10 christos Exp $");
#endif
#endif /* LIBC_SCCS and not lint */

#include "namespace.h"
#ifdef NLS
#include <limits.h>
#include <nl_types.h>
#endif

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include "extern.h"

char *
__strsignal(num, buf, buflen)
	int num;
	char *buf;
	int buflen;
{
#define	UPREFIX	"Unknown signal: %u"
	register unsigned int signum;

#ifdef NLS
	nl_catd catd ;
	catd = catopen("libc", 0);
#endif

	signum = num;				/* convert to unsigned */
	if (signum < NSIG) {
#ifdef NLS
		(void)strncpy(buf, catgets(catd, 2, signum,
		    (char *)sys_siglist[signum]), NL_TEXTMAX); 
		buf[NL_TEXTMAX - 1] = '\0';
#else
		return((char *)sys_siglist[signum]);
#endif
	} else {
#ifdef NLS
		(void)snprintf(buf, NL_TEXTMAX, 
	            catgets(catd, 1, 0xffff, UPREFIX), signum);
#else
		(void)snprintf(buf, buflen, UPREFIX, signum);
#endif
	}

#ifdef NLS
	catclose(catd);
#endif

	return buf;
}
