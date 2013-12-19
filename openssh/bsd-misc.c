
/*
 * Copyright (c) 1999-2004 Damien Miller <djm@mindrot.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>
#ifdef HAVE_SYS_SELECT_H
# include <sys/select.h>
#endif
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif

#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "xmalloc.h"
#include "bsd-misc.h"

#ifndef HAVE___PROGNAME
char *__progname;
#endif

/*
 * NB. duplicate __progname in case it is an alias for argv[0]
 * Otherwise it may get clobbered by setproctitle()
 */
char *ssh_get_progname(char *argv0)
{
#ifdef HAVE___PROGNAME
	extern char *__progname;

	return xstrdup(__progname);
#else
	char *p;

	if (argv0 == NULL)
		return ("unknown");	/* XXX */
	p = strrchr(argv0, '/');
	if (p == NULL)
		p = argv0;
	else
		p++;

	return (xstrdup(p));
#endif
}

#ifndef HAVE_INNETGR
int innetgr(const char *netgroup, const char *host, 
            const char *user, const char *domain)
{
	return (0);
}
#endif /* HAVE_INNETGR */

mysig_t
mysignal(int sig, mysig_t act)
{
#ifdef HAVE_SIGACTION
	struct sigaction sa, osa;

	if (sigaction(sig, NULL, &osa) == -1)
		return (mysig_t) -1;
	if (osa.sa_handler != act) {
		memset(&sa, 0, sizeof(sa));
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;
#ifdef SA_INTERRUPT
		if (sig == SIGALRM)
			sa.sa_flags |= SA_INTERRUPT;
#endif
		sa.sa_handler = act;
		if (sigaction(sig, &sa, NULL) == -1)
			return (mysig_t) -1;
	}
	return (osa.sa_handler);
#else
	#undef signal
	return (signal(sig, act));
#endif
}

#ifndef HAVE_ISBLANK
int
isblank(int c)
{
	return (c == ' ' || c == '\t');
}
#endif

#ifndef HAVE_GETPGID
pid_t
getpgid(pid_t pid)
{
#if defined(HAVE_GETPGRP) && !defined(GETPGRP_VOID)
	return getpgrp(pid);
#elif defined(HAVE_GETPGRP)
	if (pid == 0)
		return getpgrp();
#endif

	errno = ESRCH;
	return -1;
}
#endif
