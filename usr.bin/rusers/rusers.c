/*	$NetBSD: rusers.c,v 1.13 1997/08/24 02:40:44 lukem Exp $	*/

/*-
 *  Copyright (c) 1993 John Brezak
 *  All rights reserved.
 * 
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR `AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#ifndef lint
__RCSID("$NetBSD: rusers.c,v 1.13 1997/08/24 02:40:44 lukem Exp $");
#endif /* not lint */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <rpc/rpc.h>
#include <arpa/inet.h>
#include <err.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <utmp.h>

/*
 * For now we only try version 2 of the protocol. The current
 * version is 3 (rusers.h), but only Solaris and NetBSD seem
 * to support it currently.
 */
#include <rpcsvc/rnusers.h>	/* Old version */


#define MAX_INT 0x7fffffff
extern char *__progname;	/* from crt0.o */

struct timeval timeout = { 25, 0 };
int longopt;
int allopt;

void	allhosts __P((void));
int	main __P((int, char *[]));
void	onehost __P((char *));
void	remember_host __P((struct in_addr));
int	rusers_reply __P((char *, struct sockaddr_in *));
int	search_host __P((struct in_addr));
void	usage __P((void));

struct host_list {
	struct host_list *next;
	struct in_addr addr;
} *hosts;

int
search_host(struct in_addr addr)
{
	struct host_list *hp;
	
	if (!hosts)
		return(0);

	for (hp = hosts; hp != NULL; hp = hp->next) {
		if (hp->addr.s_addr == addr.s_addr)
			return(1);
	}
	return(0);
}

void
remember_host(struct in_addr addr)
{
	struct host_list *hp;

	if (!(hp = (struct host_list *)malloc(sizeof(struct host_list))))
		errx(1, "no memory");
	hp->addr.s_addr = addr.s_addr;
	hp->next = hosts;
	hosts = hp;
}

int
rusers_reply(char *replyp, struct sockaddr_in *raddrp)
{
	int x;
	struct hostent *hp;
	struct utmpidlearr *up = (struct utmpidlearr *)replyp;
	char *host;
	
	if (search_host(raddrp->sin_addr))
		return(0);

	if (!allopt && !up->uia_cnt)
		return(0);
	
	hp = gethostbyaddr((char *)&raddrp->sin_addr.s_addr,
			   sizeof(struct in_addr), AF_INET);
	if (hp)
		host = hp->h_name;
	else
		host = inet_ntoa(raddrp->sin_addr);
	
#define HOSTWID (int)sizeof(up->uia_arr[0]->ui_utmp.ut_host)
#define LINEWID (int)sizeof(up->uia_arr[0]->ui_utmp.ut_line)
#define NAMEWID (int)sizeof(up->uia_arr[0]->ui_utmp.ut_name)

	if (!longopt)
		printf("%-*.*s ", HOSTWID, HOSTWID, host);
	
	for (x = 0; x < up->uia_cnt; x++) {
		unsigned int minutes;
		char	date[26], idle[8];
		char	remote[HOSTWID + 3];		/* "(" host ")" \0 */
		char	local[HOSTWID + LINEWID + 2];	/* host ":" line \0 */

		if (!longopt) {
			printf("%.*s ", NAMEWID,
			    up->uia_arr[x]->ui_utmp.ut_name);
			continue;
		}

		snprintf(local, sizeof(local), "%.*s:%s",
		    HOSTWID, host, 
		    up->uia_arr[x]->ui_utmp.ut_line);

		snprintf(date, sizeof(date), "%s",
		    &(ctime((time_t *)&(up->uia_arr[x]->ui_utmp.ut_time)))[4]);

		minutes = up->uia_arr[x]->ui_idle;
		if (minutes == MAX_INT)
			strcpy(idle, "??");
		else if (minutes == 0)
			strcpy(idle, "");
		else {
			unsigned int days, hours;

			days = minutes / (24 * 60);
			minutes %= (24 * 60);
			hours = minutes / 24;
			minutes %= 24;

			if (days > 0)
				snprintf(idle, sizeof(idle), "%d d ", days);
			else if (hours > 0)
				snprintf(idle, sizeof(idle), "%2d:%02d",
				    hours, minutes);
			else
				snprintf(idle, sizeof(idle), ":%02d", minutes);
		}

		if (up->uia_arr[x]->ui_utmp.ut_host[0] != '\0')
			snprintf(remote, sizeof(remote), "(%.*s)",
			    HOSTWID, up->uia_arr[x]->ui_utmp.ut_host);
		else
			remote[0] = '\0';

		printf("%-*.*s  %-*.*s  %-12.12s  %8.8s  %s\n",
		    NAMEWID, NAMEWID, up->uia_arr[x]->ui_utmp.ut_name,
		    HOSTWID+LINEWID+1, HOSTWID+LINEWID+1, local,
		    date, idle, remote);
	}
	if (!longopt)
		putchar('\n');
	
	remember_host(raddrp->sin_addr);
	return(0);
}

void
onehost(char *host)
{
	struct utmpidlearr up;
	CLIENT *rusers_clnt;
	enum clnt_stat clnt_stat;
	struct sockaddr_in addr;
	struct hostent *hp;
	
	hp = gethostbyname(host);
	if (hp == NULL)
		errx(1, "unknown host \"%s\"", host);

	rusers_clnt = clnt_create(host, RUSERSPROG, RUSERSVERS_IDLE, "udp");
	if (rusers_clnt == NULL) {
		clnt_pcreateerror(__progname);
		exit(1);
	}

	memset((char *)&up, 0, sizeof(up));
	clnt_stat = clnt_call(rusers_clnt, RUSERSPROC_NAMES, xdr_void, NULL,
	    xdr_utmpidlearr, &up, timeout);
	if (clnt_stat != RPC_SUCCESS)
		errx(1, "%s", clnt_sperrno(clnt_stat));
	addr.sin_addr.s_addr = *(int *)hp->h_addr;
	rusers_reply((char *)&up, &addr);
}

void
allhosts(void)
{
	struct utmpidlearr up;
	enum clnt_stat clnt_stat;

	memset((char *)&up, 0, sizeof(up));
	clnt_stat = clnt_broadcast(RUSERSPROG, RUSERSVERS_IDLE,
	    RUSERSPROC_NAMES, xdr_void, NULL, xdr_utmpidlearr,
	    (char *)&up, rusers_reply);
	if (clnt_stat != RPC_SUCCESS && clnt_stat != RPC_TIMEDOUT)
		errx(1, "%s", clnt_sperrno(clnt_stat));
}

void
usage(void)
{
	fprintf(stderr, "Usage: %s [-la] [hosts ...]\n", __progname);
	exit(1);
}

int
main(int argc, char *argv[])
{
	int ch;
	extern int optind;
	
	while ((ch = getopt(argc, argv, "al")) != -1)
		switch (ch) {
		case 'a':
			allopt++;
			break;
		case 'l':
			longopt++;
			break;
		default:
			usage();
			/*NOTREACHED*/
		}

	setlinebuf(stdout);
	if (argc == optind)
		allhosts();
	else {
		for (; optind < argc; optind++)
			(void) onehost(argv[optind]);
	}
	exit(0);
}
