/********************************************************************************
$Id: hscp.cpp 102 2010-12-26 13:47:53Z bunpojpn $

hscp.cpp v 0.9.20 2010-12-25
Copyright (c) 2009,2010 RCCS technical team of IMS,
   Fumiyasu Mizutani,
   Fumitsuna Teshima, Masataka Sawa,
   Shigeki Naitoh,    Jun-ichi Matsuo,
   Kensuke Iwahashi,  Takakazu Nagaya.
All rights reserved.
We special thanks to
   Hironori Kogawa (Hitachi, Ltd., for the first try to merge UDT into scp),
   UDT distributer (the board of trustees of the University of Illinois),
   OpenSSH distributers, and other open source distributers.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of
    conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation and/or
    other materials provided with the distribution.
  * Neither the name of the "NINS (National Institutes of Natural Sciences),
    IMS (Institute for Molecular Science)" nor the names of its contributors may be
    used to endorse or promote products derived from this software without specific
    prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANT ABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*********************************************************************************/
/* Original copyrights of scp.c is under this line,                             */
/********************************************************************************/
/* $OpenBSD: scp.c,v 1.165 2009/12/20 07:28:36 guenther Exp $ */
/*
 * scp - secure remote copy.  This is basically patched BSD rcp which
 * uses ssh to do the data transfer (instead of using rcmd).
 *
 * NOTE: This version should NOT be suid root.  (This uses ssh to
 * do the transfer and ssh has the necessary privileges.)
 *
 * 1995 Timo Rinne <tri@iki.fi>, Tatu Ylonen <ylo@cs.hut.fi>
 *
 * As far as I am concerned, the code I have written for this software
 * can be used freely for any purpose.  Any derived versions of this
 * software must be clearly marked as such, and if the derived work is
 * incompatible with the protocol description in the RFC file, it must be
 * called by a name other than "ssh" or "Secure Shell".
 */
/*
 * Copyright (c) 1999 Theo de Raadt.  All rights reserved.
 * Copyright (c) 1999 Aaron Campbell.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * Parts from:
 *
 * Copyright (c) 1983, 1990, 1992, 1993, 1995
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
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
 *
 */
/********************************************************************************/
#define _LARGEFILE_SOURCE
#define _FILE_OFFSET_BITS 64

#include <iostream>
using namespace std;
#include "hscp.h"
#include "udtscp.h"
#include <sstream>
#include <arpa/inet.h>

extern "C" {
#include "includes.h"

#include <sys/types.h>
#include <sys/param.h>
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef HAVE_POLL_H
#include <poll.h>
#else
# ifdef HAVE_SYS_POLL_H
#  include <sys/poll.h>
# endif
#endif
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#include <sys/wait.h>
#include <sys/uio.h>

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pwd.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
/*#include <getopt.h>*/
#if defined(HAVE_STRNVIS) && defined(HAVE_VIS_H)
#include <vis.h>
#endif

#include "xmalloc.h"
#include "atomicio.h"
#include "pathnames.h"
#include "log.h"
#include "misc.h"
//#include "progressmeter.h"
}

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif  /* PATH_MAX */

extern char *__progname;

#define COPY_BUFLEN	16384

int do_cmd(char *host, char *remuser, char *cmd, int *fdin, int *fdout);
void lostconn(int);

void bwlimit(int);

/* Struct for addargs */
arglist args;

/* Bandwidth limit */
long limit_rate = 0;

/* Name of current file being transferred. */
char *curfile;

/* This is set to non-zero to enable verbose mode. */
int verbose_mode = 0;

/* This is set to zero if the progressmeter is not desired. */
int showprogress = 0;

/* This is the program to execute for the secured connection. ("ssh" or -S) */
const char *ssh_program = "ssh";
const char *rsh_program = "rsh";
int  run_rcommand = 0;
int  server_accept = 0;
int  ssh_verbose = 1;
int  ssh_quiet = 1;
int  ipvx = 4;
int  show_unit = 1;
int  hscp_protocol = 1;

/* This is used to store the pid of ssh_program */
pid_t do_cmd_pid = -1;

/* use UDT transfer mode */
int udt_mode = 1;

/* server flag */
int svflag = 0;
const char *nodes = NODES;
const char *nodec = NODEC;
char *nodemode = (char *)nodec;
const char *atomicioWrite = "atomicio write:";
const char *null = "NULL";

/* configure file path */
char cpath[PATH_MAX];
/* extend server path */
char epath[PATH_MAX];

static void
killchild(int signo)
{
	if (do_cmd_pid > 1) {
		kill(do_cmd_pid, signo ? signo : SIGTERM);
		waitpid(do_cmd_pid, NULL, 0);
	}

	if (signo)
		_exit(1);
	exit(1);
}

static void
suspchild(int signo)
{
        int status;

        if (do_cmd_pid > 1) {
                kill(do_cmd_pid, signo);
                while (waitpid(do_cmd_pid, &status, WUNTRACED) == -1 &&
                    errno == EINTR)
                        ;
                kill(getpid(), SIGSTOP);
        }
}

static int
do_local_cmd(arglist *a)
{
	const char *ftitle = "[do_local_cmd]";
	u_int i;
	int status;
	pid_t pid;

	if (a->num == 0)
		fatal("do_local_cmd: no arguments");

	if (verbose_mode) {
		fprintf(stderr, "%s%s Executing:", nodemode, ftitle);
		for (i = 0; i < a->num; i++)
			fprintf(stderr, " %s", a->list[i]);
		fprintf(stderr, "\n");
	}
	if ((pid = fork()) == -1)
		fatal("do_local_cmd: fork: %s", strerror(errno));

	if (pid == 0) {
		execvp(a->list[0], a->list);
		perror(a->list[0]);
		exit(1);
	}

	do_cmd_pid = pid;
	signal(SIGTERM, killchild);
	signal(SIGINT, killchild);
	signal(SIGHUP, killchild);
	signal(SIGPIPE, lostconn);

	while (waitpid(pid, &status, 0) == -1)
		if (errno != EINTR)
			fatal("do_local_cmd: waitpid: %s", strerror(errno));

	do_cmd_pid = -1;

	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		return (-1);

	return (0);
}

/*
 * This function executes the given command as the specified user on the
 * given host.  This returns < 0 if execution fails, and >= 0 otherwise. This
 * assigns the input and output file descriptors on success.
 */

int
do_cmd(char *host, char *remuser, char *cmd, int *fdin, int *fdout)
{
	const char *ftitle = "[do_cmd]";
	int pin[2], pout[2], reserved[2];

	if (verbose_mode) fprintf(stderr,
		"%s%s Executing: program %s host %s, user %s, command %s\n",
			nodemode, ftitle, ssh_program, host,
			remuser ? remuser : "(unspecified)", cmd);

	/*
	 * Reserve two descriptors so that the real pipes won't get
	 * descriptors 0 and 1 because that will screw up dup2 below.
	 */
	if (pipe(reserved) < 0)
		fatal("pipe: %s", strerror(errno));

	/* Create a socket pair for communicating with ssh. */
	if (pipe(pin) < 0)
		fatal("pipe: %s", strerror(errno));
	if (pipe(pout) < 0)
		fatal("pipe: %s", strerror(errno));

	/* Free the reserved descriptors. */
	close(reserved[0]);
	close(reserved[1]);

        signal(SIGTSTP, suspchild);
        signal(SIGTTIN, suspchild);
        signal(SIGTTOU, suspchild);

	/* Fork a child to execute the command on the remote host using ssh. */
	do_cmd_pid = fork();
	if (do_cmd_pid == 0) {
		/* Child. */
		close(pin[1]);
		close(pout[0]);
		dup2(pin[0], 0);
		dup2(pout[1], 1);
		close(pin[0]);
		close(pout[1]);

		replacearg(&args, 0, (char *)"%s", ssh_program);
		if (remuser != NULL){
			addargs(&args, (char *)"-l");
			addargs(&args, (char *)"%s", remuser);
		}
		if (!run_rcommand){
			if (verbose_mode && ssh_verbose) addargs(&args, (char *)"-v");
			else if (ssh_quiet) addargs(&args, (char *)"-q");
			addargs(&args, (char *)"--");
		}
		addargs(&args, (char *)"%s", host);
		addargs(&args, (char *)"%s", cmd);

		execvp(ssh_program, args.list);
		perror(ssh_program);
		exit(1);
	} else if (do_cmd_pid == -1) {
		fatal("fork: %s", strerror(errno));
	}
	/* Parent.  Close the other side, and return the local side. */
	close(pin[0]);
	*fdout = pin[1];
	close(pout[1]);
	*fdin = pout[0];
	signal(SIGTERM, killchild);
	signal(SIGINT, killchild);
	signal(SIGHUP, killchild);
	signal(SIGPIPE, lostconn);
	return 0;
}

typedef struct {
	size_t cnt;
	char *buf;
} BUF;

BUF *allocbuf(BUF *, int, int);
int okname(char *);
void run_err(const char *,...);
void verifydir(char *);

struct passwd *pwd;
uid_t userid;
int errs, remin, remout;
int pflag, iamremote, iamrecursive, targetshouldbedirectory;

#define	CMDNEEDS	64
char cmd[PATH_MAX+CMDNEEDS];		/* must hold "rcp -r -p -d\0" */

int response(const char *);
int response_v(const char *);
void rsource(char *, struct stat *, UDTScp&);
void sink(int, char *[], UDTScp&, int);
void source(int, char *[], UDTScp&);
void tolocal(int, char *[]);
void toremote(char *, int, char *[]);
size_t scpio(ssize_t (*)(int, void *, size_t), int, void *, size_t, off_t *);
void usage(void);
void title_display(void);
//void title_display_server(void);

int
main(int argc, char **argv)
{
	const char *ftitle = "[main]";
	char limitbuf[32];
	int ch, fflag, tflag, status, n;
	double speed;
	char *targ, *endp, **newargv;
	extern char *optarg;
	extern int optind;

	/* Ensure that fds 0, 1 and 2 are open or directed to /dev/null */
	sanitise_stdfd();

	/* Copy argv, because we modify it */
	newargv = (char **)xcalloc(MAX(argc + 1, 1), sizeof(*newargv));
	for (n = 0; n < argc; n++)
		newargv[n] = xstrdup(argv[n]);
	argv = newargv;

	__progname = ssh_get_progname(argv[0]);

	memset(&args, '\0', sizeof(args));
	args.list = NULL;
	addargs(&args, (char *)"%s", ssh_program);

	if (getenv(ENVCONF) != NULL) strcpy(cpath, getenv(ENVCONF));	
        if (strlen(cpath) == 0 || access(cpath, F_OK)){
          strncpy(cpath, HSCP_CONF_DIR, sizeof(cpath)-1);
          strncat(cpath, HSCP_CONF, sizeof(cpath)-1);
          if (access(cpath, F_OK)){
            strcpy(cpath, "");
          }
        }
        if (strlen(cpath) == 0){
          fprintf(stderr, "HSCP [WARNING] Configulation file is not found.\n");
        }
        else {
          ifstream ifs(cpath);
          string line;
          char *p, *cbuf;
          while (getline(ifs, line)){
            if (line.find("SshPath") == 0) {
              cbuf = new char [line.size() + 1];
              strcpy(cbuf, line.c_str());
              p = strtok(cbuf, " ");
              p = strtok(NULL, " ");
              ssh_program = p;
            }
            if (line.find("RshPath") == 0) {
              cbuf = new char [line.size() + 1];
              strcpy(cbuf, line.c_str());
              p = strtok(cbuf, " ");
              p = strtok(NULL, " ");
              rsh_program = p;
            }
            if (line.find("ServerAccept") == 0) {
              cbuf = new char [line.size() + 1];
              strcpy(cbuf, line.c_str());
              p = strtok(cbuf, " ");
              p = strtok(NULL, " ");
              server_accept = atoi(p);
            }
            if (line.find("SshVerbose") == 0) {
              cbuf = new char [line.size() + 1];
              strcpy(cbuf, line.c_str());
              p = strtok(cbuf, " ");
              p = strtok(NULL, " ");
              ssh_verbose = atoi(p);
            }
          }
          ifs.close();
        }

	fflag = tflag = 0;
	strcpy(epath, "");
//        int option_index = 0;
//        static struct option long_options[] = {
//          {UDTSBS,1,0,0},
//          {UDTRBS,1,0,0},
//          {UDPSBS,1,0,0},
//          {UDPRBS,1,0,0},
//          {0,0,0,0}
//        };
//	while ((ch = getopt_long(argc, argv, "dfl:prtvBCc:i:P:q1246S:o:F:sI:R:", long_options, &option_index)) != -1)
	while ((ch = getopt(argc, argv, "dfl:prtvBCc:i:P:q1246S:o:F:sI:RE:U")) != -1)
		switch (ch) {
		/* User-visible flags. */
		case '1':
		case '2':
		case '4':
		case 'C':
			addargs(&args, (char *)"-%c", ch);
			break;
		case '6':
			ipvx = 6;
			addargs(&args, (char *)"-%c", ch);
			break;
		case 'o':
		case 'c':
		case 'i':
		case 'F':
			addargs(&args, (char *)"-%c", ch);
			addargs(&args, (char *)"%s", optarg);
			break;
		case 'P':
			addargs(&args, (char *)"-p");
			addargs(&args, (char *)"%s", optarg);
			break;
		case 'B':
			addargs(&args, (char *)"-oBatchmode yes");
			break;
		case 'l':
			speed = strtod(optarg, &endp);
			if (speed <= 0) usage();
			if      (*endp == '\0') ;
			else if (*endp ==  'k') ;
			else if (*endp ==  'K') ;
			else if (*endp ==  'M') speed *= 1000;
			else if (*endp ==  'G') speed *= 1000 * 1000;
			else usage();
			limit_rate = (long)(speed * 1000);
			sprintf(limitbuf, " -l %.3f", speed);
			break;
		case 'p':
			pflag = 1;
			break;
		case 'r':
			iamrecursive = 1;
			break;
		case 'S':
			ssh_program = xstrdup(optarg);
			break;
		case 'v':
			verbose_mode = 1;
			break;
		case 'q':
			addargs(&args, (char *)"-q");
			showprogress = 9;
			break;

		/* Server options. */
		case 'd':
			targetshouldbedirectory = 1;
			break;
		case 'f':	/* "from" */
			iamremote = 1;
			fflag = 1;
			break;
		case 't':	/* "to" */
			iamremote = 1;
			tflag = 1;
#ifdef HAVE_CYGWIN
			setmode(0, O_BINARY);
#endif
			break;
                /* hscp append options */
		case 's':
#ifdef SCPXFER
			udt_mode = 0;
#endif
			break;
		case 'I':
			showprogress = (int)strtol(optarg, &endp,10);
			if (showprogress < 1 || showprogress > 3 || *endp != '\0')
				usage();
			break;
		case 'R':
			ssh_program = rsh_program;
			run_rcommand = 1;
			break;
		case 'E':
			strncpy(epath, optarg, sizeof(epath)-1);
			if (epath[strlen(epath) - 1] != '/')
				strncat(epath, "/", sizeof(epath)-1);
			break;
		case 'U':
			show_unit = 0;
			break;
                /* rcp options */
		default:
			usage();
		}
	argc -= optind;
	argv += optind;
	if ((pwd = getpwuid(userid = getuid())) == NULL)
		fatal("unknown user %u", (u_int) userid);

//	if (!isatty(STDOUT_FILENO)) showprogress = 9;

	remin = STDIN_FILENO;
	remout = STDOUT_FILENO;
	if (fflag || tflag) {
		if (!server_accept){
			fprintf(stderr, "Sorry, server is not acceptable.\n");
			exit(999);
		}
                svflag = 1;
		nodemode = (char *)nodes;
        }
        if (verbose_mode)
		fprintf(stderr, "%s%s Read config: %s\n", nodemode, ftitle, cpath);

	if (fflag) {
		const char *fgtitle = " [fflag]";
		UDTScp udtserver;
		udtserver.set_showprogress(999);
		udtserver.set_show_unit(show_unit);
		udtserver.set_limit_rate(limit_rate);
		udtserver.set_verbose_mode(verbose_mode);
		udtserver.set_server_flag(svflag, nodemode);
		udtserver.set_config_path(cpath);
		if (ipvx == 6) udtserver.set_ipv6();
		if (verbose_mode) title_display();
		if (udt_mode) {
//			if (!udtserver.listen()) exit(errs != 0);
			if (!udtserver.listen()) exit(1);
			int port = udtserver.get_assigned_port();
	
			char buf[2048];
			if (verbose_mode)
				fprintf(stderr, "%s%s%s Sending port number: %d\n",
					nodemode, ftitle, fgtitle, port);
			(void) snprintf(buf, sizeof buf, "P%d 1\n", port);
			(void) atomicio(vwrite, remout, buf, strlen(buf));
			if (verbose_mode) fprintf(stderr, "%s%s%s %s %s",
				nodemode, ftitle, fgtitle, atomicioWrite, buf);
			response_v(ftitle);
			udtserver.set_hscp_protocol(hscp_protocol);

//			if (!udtserver.accept()) exit(errs != 0);
			if (!udtserver.accept()) exit(1);
		}

		/* Follow "protocol", send data. */
		response(ftitle);
		source(argc, argv, udtserver);
		if (udt_mode) {
			udtserver.disconnect();
		}
		exit(errs != 0);
	}
	else if (tflag) {
		const char *fgtitle = " [tflag]";
		(void) atomicio(vwrite, remout, (void *)"", 1);
		if (verbose_mode) fprintf(stderr, "%s%s%s %s %s\n",
			nodemode, ftitle, fgtitle, atomicioWrite, null);

		UDTScp udtserver;
		udtserver.set_showprogress(999);
		udtserver.set_show_unit(show_unit);
		udtserver.set_limit_rate(0);
		udtserver.set_verbose_mode(verbose_mode);
		udtserver.set_server_flag(svflag, nodemode);
		udtserver.set_config_path(cpath);
		if (ipvx == 6) udtserver.set_ipv6();
		if (verbose_mode) title_display();
		if (udt_mode) {
//			if (!udtserver.listen()) exit(errs != 0);
			if (!udtserver.listen()) exit(1);
			int port = udtserver.get_assigned_port();

			char buf[2048];
			if (verbose_mode)
				fprintf(stderr, "%s%s%s Sending port number: %d\n",
					nodemode, ftitle, fgtitle, port);
			(void) snprintf(buf, sizeof buf, "P%d 1\n", port);
			(void) atomicio(vwrite, remout, buf, strlen(buf));
			if (verbose_mode) fprintf(stderr, "%s%s%s %s %s",
				nodemode, ftitle, fgtitle, atomicioWrite, buf);
			response_v(ftitle);
			udtserver.set_hscp_protocol(hscp_protocol);
		
//			if (!udtserver.accept()) exit(errs != 0);
			if (!udtserver.accept()) exit(1);
		}

		/* Receive data. */
		sink(argc, argv, udtserver, 0);
		if (udt_mode) {
			udtserver.disconnect();
		}
		exit(errs != 0);
	}
	if (argc < 2){
		usage();
        }
	else {
		if (verbose_mode) title_display();
        }
	if (argc > 2)
		targetshouldbedirectory = 1;

	if (!run_rcommand){
		addargs(&args, (char *)"-x");
		addargs(&args, (char *)"-oForwardAgent no");
//		addargs(&args, (char *)"-oPermitLocalCommand no");
		addargs(&args, (char *)"-oClearAllForwardings yes");
                if (verbose_mode && ssh_verbose) addargs(&args, (char *)"-v");
	}

	remin = remout = -1;
	do_cmd_pid = -1;
	/* Command to be executed on remote system using "ssh". */
	(void) snprintf(cmd, sizeof cmd, "%shscp%s%s%s%s%s%s",
	    epath,
	    verbose_mode ? " -v" : "",
	    iamrecursive ? " -r" : "", pflag ? " -p" : "",
	    targetshouldbedirectory ? " -d" : "",
 	    limit_rate ? limitbuf : "",
	    ipvx == 6 ? " -6" : "");

	(void) signal(SIGPIPE, lostconn);

	if ((targ = colon(argv[argc - 1])))	/* Dest is remote host. */
		toremote(targ, argc, argv);
	else {
		if (targetshouldbedirectory)
			verifydir(argv[argc - 1]);
		tolocal(argc, argv);	/* Dest is local host. */
	}
	/*
	 * Finally check the exit status of the ssh process, if one was forked
	 * and no error has occurred yet
	 */
	if (do_cmd_pid != -1 && errs == 0) {
		if (remin != -1)
		    (void) close(remin);
		if (remout != 0)
		    (void) close(remout);
		if (waitpid(do_cmd_pid, &status, 0) == -1)
			errs = 1;
		else {
			if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
				errs = 1;
		}
	}
	exit(errs != 0);
}

/*
 * atomicio-like wrapper that also applies bandwidth limits and updates
 * the progressmeter counter.
 */
size_t
scpio(ssize_t (*f)(int, void *, size_t), int fd, void *_p, size_t l, off_t *c)
{
	u_char *p = (u_char *)_p;
	size_t offset;
	ssize_t r;
	struct pollfd pfd;

	pfd.fd = fd;
	pfd.events = f == read ? POLLIN : POLLOUT;
	for (offset = 0; offset < l;) {
		r = f(fd, p + offset, l - offset);
		if (r == 0) {
			errno = EPIPE;
			return offset;
		}
		if (r < 0) {
			if (errno == EINTR)
				continue;
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				(void)poll(&pfd, 1, -1); /* Ignore errors */
				continue;
			}
			return offset;
		}
		offset += (size_t)r;
		*c += (off_t)r;
		if (limit_rate)
			bwlimit(r);
	}
	return offset;
}

void
toremote(char *targ, int argc, char **argv)
{
	const char *ftitle = "[toremote]";
	char *bp, *host, *src, *suser, *thost, *tuser, *arg;
	arglist alist;
	int i;

	memset(&alist, '\0', sizeof(alist));
	alist.list = NULL;

	*targ++ = 0;
	if (*targ == 0)
		targ = (char *)".";

	arg = xstrdup(argv[argc - 1]);
	if ((thost = strrchr(arg, '@'))) {
		/* user@host */
		*thost++ = 0;
		tuser = arg;
		if (*tuser == '\0')
			tuser = NULL;
	} else {
		thost = arg;
		tuser = NULL;
	}

	if (tuser != NULL && !okname(tuser)) {
		xfree(arg);
		return;
	}
	for (i = 0; i < argc - 1; i++) {
		src = colon(argv[i]);
		if (src && udt_mode){	/* remote to remote */
			fprintf(stderr, "ERROR: hscp can't send remote to remote.\n");
			exit(1);
		}
	}

	UDTScp udtclient;
	int udtconnected = 0;
	for (i = 0; i < argc - 1; i++) {
		src = colon(argv[i]);
		if (src) {	/* remote to remote */
			freeargs(&alist);
			addargs(&alist, (char *)"%s", ssh_program);
			if (!run_rcommand){
				if (verbose_mode && ssh_verbose) addargs(&alist, (char *)"-v");
				addargs(&alist, (char *)"-x");
				addargs(&alist, (char *)"-oClearAllForwardings yes");
				addargs(&alist, (char *)"-oForwardAgent no");
			}
			addargs(&alist, (char *)"-n");

			*src++ = 0;
			if (*src == 0)
				src = (char *)".";
			host = strrchr(argv[i], '@');

			if (host) {
				*host++ = 0;
				host = cleanhostname(host);
				suser = argv[i];
				if (*suser == '\0')
					suser = pwd->pw_name;
				else if (!okname(suser))
					continue;
				addargs(&alist, (char *)"-l");
				addargs(&alist, (char *)"%s", suser);
			} else {
				host = cleanhostname(argv[i]);
			}
			addargs(&alist, (char *)"--");
			addargs(&alist, (char *)"%s", host);
			addargs(&alist, (char *)"%s", cmd);
			addargs(&alist, (char *)"%s", src);
			addargs(&alist, (char *)"%s%s%s:%s",
			    tuser ? tuser : "", tuser ? "@" : "",
			    thost, targ);
			if (do_local_cmd(&alist) != 0)
				errs = 1;
		} else {	/* local to remote */
			if (remin == -1) {
				xasprintf(&bp, "%s -t -- %s", cmd, targ);
				host = cleanhostname(thost);
				if (do_cmd(host, tuser, bp, &remin,
				    &remout) < 0)
					exit(1);
				if (response(ftitle) < 0)
					exit(1);
				(void) xfree(bp);

				udtclient.set_showprogress(showprogress);
				udtclient.set_show_unit(show_unit);
			        udtclient.set_limit_rate(limit_rate);
				udtclient.set_verbose_mode(verbose_mode);
			        udtclient.set_server_flag(svflag, nodemode);
			        udtclient.set_config_path(cpath);
				if (ipvx == 6) udtclient.set_ipv6();
				if (udt_mode) {
				  char bbuf[2048], *endptr;
				  char bufport[1024];
				  int port;
				  if (!iamremote) {
				    char ch, *cp, *why;
				    cp = bbuf;
				    int rc;
				    if (rc = atomicio(read, remin, cp, 1) != 1) {
				      return;
				    }
				    do {
				      if (atomicio(read, remin, &ch, sizeof(ch)) != sizeof(ch))
				        cout << "lost connection" << endl;
				      *cp++ = ch;
				    } while (cp < &bbuf[sizeof(bbuf) - 1] && ch != '\n');
				    cp = bbuf;
//				    port = strtol(cp, NULL, 10);
				    port = strtol(cp, &endptr, 10);
				    hscp_protocol = strtol(endptr, NULL, 10);
				    if (verbose_mode){
				      fprintf(stderr, "%s%s Recieve port number: %d <- %s", 
				        nodemode, ftitle, port, bbuf);
				      fprintf(stderr, "%s%s Recieve protocol number: %d <- %s", 
				        nodemode, ftitle, hscp_protocol, bbuf);
				    }
				    sprintf(bufport, "%d", port);
				    if (hscp_protocol == 1){
				      (void) atomicio(vwrite, remout, (void *)"1\n", 2);
				      if (verbose_mode) fprintf(stderr, "%s%s %s %s\n",
				         nodemode, ftitle, atomicioWrite, "1");
				    }
				    if (hscp_protocol == 0){
				      (void) atomicio(vwrite, remout, (void *)"", 1);
				      if (verbose_mode) fprintf(stderr, "%s%s %s %s\n",
				         nodemode, ftitle, atomicioWrite, null);
				    }
				    udtclient.set_hscp_protocol(hscp_protocol);
				  }
				  if (udtclient.connect(host, (char *)bufport) == 0){
				    run_err("UDT couldn't connect to %s", host);
				    xfree(arg);
				    return;
                                  }
				  udtconnected = 1;
				}
			
			}
			source(1, argv + i, udtclient);
		}
	}
	if (udt_mode && udtconnected) {
		udtclient.disconnect();
	}
	xfree(arg);
}

void
tolocal(int argc, char **argv)
{
	const char *ftitle = "[tolocal]";
	char *bp, *host, *src, *suser;
	arglist alist;
	int i;

	memset(&alist, '\0', sizeof(alist));
	alist.list = NULL;

	for (i = 0; i < argc - 1; i++) {
		if (!(src = colon(argv[i]))) {	/* Local to local. */
			freeargs(&alist);
			addargs(&alist, (char *)"%s", _PATH_CP);
			if (iamrecursive)
				addargs(&alist, (char *)"-r");
			if (pflag)
				addargs(&alist, (char *)"-p");
			addargs(&alist, (char *)"--");
			addargs(&alist, (char *)"%s", argv[i]);
			addargs(&alist, (char *)"%s", argv[argc-1]);
			if (do_local_cmd(&alist))
				++errs;
			continue;
		}
		*src++ = 0;
		if (*src == 0)
			src = (char *)".";
		if ((host = strrchr(argv[i], '@')) == NULL) {
			host = argv[i];
			suser = NULL;
		} else {
			*host++ = 0;
			suser = argv[i];
			if (*suser == '\0')
				suser = pwd->pw_name;
		}
		host = cleanhostname(host);
		xasprintf(&bp, "%s -f -- %s", cmd, src);
		if (do_cmd(host, suser, bp, &remin, &remout) < 0) {
			(void) xfree(bp);
			++errs;
			continue;
		}
		xfree(bp);

		UDTScp udtclient;
		udtclient.set_showprogress(showprogress);
		udtclient.set_show_unit(show_unit);
		udtclient.set_limit_rate(limit_rate);
		udtclient.set_verbose_mode(verbose_mode);
		udtclient.set_server_flag(svflag, nodemode);
		udtclient.set_config_path(cpath);
		if (ipvx == 6) udtclient.set_ipv6();
		if (udt_mode) {
			char bbuf[2048], *endptr;
			char bufport[1024];
			int port;
			if (!iamremote) {
				char ch, *cp, *why;
				cp = bbuf;
				int rc;
				if (rc = atomicio(read, remin, cp, 1) != 1) {
					return;
				}
				do {
				  if (atomicio(read,remin,&ch,sizeof(ch)) != sizeof(ch))
				    cout << "lost connection" << endl;
				  *cp++ = ch;
				} while (cp < &bbuf[sizeof(bbuf) - 1] && ch != '\n');
				cp = bbuf;
//				port = strtol(cp, NULL, 10);
				port = strtol(cp, &endptr, 10);
				hscp_protocol = strtol(endptr, NULL, 10);
				if (verbose_mode){
				  fprintf(stderr, "%s%s Send port number: %d <- %s",
				        nodemode, ftitle, port, bbuf);
				  fprintf(stderr, "%s%s Send protocol number: %d <- %s",
				        nodemode, ftitle, hscp_protocol, bbuf);
				}
				sprintf(bufport, "%d", port);
				if (hscp_protocol == 0){
				  (void) atomicio(vwrite, remout, (void *)"", 1);
				  if (verbose_mode) fprintf(stderr, "%s%s %s %s\n",
					nodemode, ftitle, atomicioWrite, null);
				}
				if (hscp_protocol == 1){
				  (void) atomicio(vwrite, remout, (void *)"1\n", 2);
				  if (verbose_mode) fprintf(stderr, "%s%s %s %s\n",
					nodemode, ftitle, atomicioWrite, "1");
				}
				udtclient.set_hscp_protocol(hscp_protocol);
			}
			if (udtclient.connect(host, (char *)bufport) == 0){
				run_err("UDT couldn't connect to %s.", host);
				return;
                        };
	
		}

		sink(1, argv + argc - 1, udtclient, 1);

		if (udt_mode) {
			udtclient.disconnect();
		}

		(void) close(remin);
		remin = remout = -1;
	}
}

void
source(int argc, char **argv, UDTScp& udtscp)
{
	const char *ftitle = "[source]";
	struct stat stb;
	static BUF buffer;
	BUF *bp;
	off_t i, statbytes;
	size_t amt, filesize;
	int fd = -1, haderr, indx;
	char *last, *name, buf[2048], encname[MAXPATHLEN];
	int len;

	if (verbose_mode) fprintf(stderr, "%s%s index: %d\n", nodemode, ftitle, argc);
	for (indx = 0; indx < argc; ++indx) {
		name = argv[indx];
		if (verbose_mode)
			fprintf(stderr, "%s%s Name: %s\n", nodemode, ftitle, name);
		statbytes = 0;
		len = strlen(name);
		while (len > 1 && name[len-1] == '/')
			name[--len] = '\0';
		if ((fd = open(name, O_RDONLY|O_NONBLOCK, 0)) < 0)
			goto syserr;
		if (strchr(name, '\n') != NULL) {
			strnvis(encname, name, sizeof(encname), VIS_NL);
			name = encname;
		}
		if (fstat(fd, &stb) < 0) {
syserr:			run_err("%s: %s", name, strerror(errno));
			goto next;
		}
		if (stb.st_size < 0) {
			run_err("%s: %s", name, "Negative file size");
			goto next;
		}
		unset_nonblock(fd);
		switch (stb.st_mode & S_IFMT) {
		case S_IFREG:
			break;
		case S_IFDIR:
			if (iamrecursive) {
				rsource(name, &stb, udtscp);
				goto next;
			}
			/* FALLTHROUGH */
		default:
			run_err("%s: not a regular file", name);
			goto next;
		}
		if ((last = strrchr(name, '/')) == NULL)
			last = name;
		else
			++last;
		curfile = last;
		if (pflag) {
			/*
			 * Make it compatible with possible future
			 * versions expecting microseconds.
			 */
			(void) snprintf(buf, sizeof buf, "T%lu 0 %lu 0\n",
			    (u_long) (stb.st_mtime < 0 ? 0 : stb.st_mtime),
			    (u_long) (stb.st_atime < 0 ? 0 : stb.st_atime));
			if (verbose_mode) 
				fprintf(stderr, "%s%s File mtime %ld atime %ld\n",
					nodemode, ftitle,
					(long)stb.st_mtime, (long)stb.st_atime);
			if (verbose_mode)
				fprintf(stderr, "%s%s File timestamps: %s\n",
				    nodemode, ftitle, buf);
			(void) atomicio(vwrite, remout, buf, strlen(buf));
			if (verbose_mode) fprintf(stderr, "%s%s %s %s",
				nodemode, ftitle, atomicioWrite, buf);
			if (response(ftitle) < 0)
				goto next;
		}
#define	FILEMODEMASK	(S_ISUID|S_ISGID|S_IRWXU|S_IRWXG|S_IRWXO)
		snprintf(buf, sizeof buf, "C%04o %lld %s\n",
		    (u_int) (stb.st_mode & FILEMODEMASK),
		    (long long)stb.st_size, last);
		filesize = (size_t)stb.st_size;
		if (verbose_mode) fprintf(stderr, "%s%s Sending file modes: %s",
			nodemode, ftitle, buf); 
		(void) atomicio(vwrite, remout, buf, strlen(buf));
		if (verbose_mode) fprintf(stderr, "%s%s %s %s",
			nodemode, ftitle, atomicioWrite, buf);
		if (response(ftitle) < 0)
			goto next;

		if ((bp = allocbuf(&buffer, fd, COPY_BUFLEN)) == NULL) {
next:			if (fd != -1) {
				(void) close(fd);
				fd = -1;
			}
			continue;
		}
	
#ifdef SCPXFER
		if (!udt_mode) {
			if (showprogress)
				start_progress_meter(curfile, stb.st_size, &statbytes);
			set_nonblock(remout);
			for (haderr = i = 0; i < stb.st_size; i += bp->cnt) {
				amt = bp->cnt;
				if (i + (off_t)amt > stb.st_size)
					amt = stb.st_size - i;
				if (!haderr) {
					if (atomicio(read, fd, bp->buf, amt) != amt)
						haderr = errno;
				}
				/* Keep writing after error to retain sync */
				if (haderr) {
					(void)atomicio(vwrite, remout, bp->buf, amt);
					if (verbose_mode) fprintf(stderr, "%s%s %s %s\n",
					    nodemode, ftitle, atomicioWrite, bp->buf);
					continue;
				}
				if (scpio(vwrite, remout, bp->buf, amt,
				    &statbytes) != amt)
					haderr = errno;
			}
			unset_nonblock(remout);
			if (showprogress)
				stop_progress_meter();
	
			if (fd != -1) {
				if (close(fd) < 0 && !haderr)
			haderr = errno;
				fd = -1;
			}
		}
#endif

		if (udt_mode) {
			close(fd);
			udtscp.sendfile(name, filesize);
			haderr = 0;
		}

		if (!haderr){
			(void) atomicio(vwrite, remout, (void *)"", 1);
			if (verbose_mode) fprintf(stderr, "%s%s %s %s\n",
				nodemode, ftitle, atomicioWrite, null);
		}
//		else
//			run_err("%s: %s", name, strerror(haderr));
		response(ftitle);
	}
}

void
rsource(char *name, struct stat *statp, UDTScp& udtscp)
{
	const char *ftitle = "[rsource]";
	DIR *dirp;
	struct dirent *dp;
	char *last, *vect[1], path[1100];

	if (!(dirp = opendir(name))) {
		run_err("%s: %s", name, strerror(errno));
		return;
	}
	last = strrchr(name, '/');
	if (last == 0)
		last = name;
	else
		last++;
	if (pflag) {
		(void) snprintf(path, sizeof(path), "T%lu 0 %lu 0\n",
		    (u_long) statp->st_mtime,
		    (u_long) statp->st_atime);
		(void) atomicio(vwrite, remout, path, strlen(path));
		if (verbose_mode) fprintf(stderr, "%s%s %s %s\n",
			nodemode, ftitle, atomicioWrite, path);
		if (response(ftitle) < 0) {
			closedir(dirp);
			return;
		}
	}
	(void) snprintf(path, sizeof path, "D%04o %d %.1024s\n",
	    (u_int) (statp->st_mode & FILEMODEMASK), 0, last);
	(void) atomicio(vwrite, remout, path, strlen(path));
	if (verbose_mode) fprintf(stderr, "%s%s %s %s\n",
		nodemode, ftitle, atomicioWrite, path);
	if (response(ftitle) < 0) {
		closedir(dirp);
		return;
	}
	while ((dp = readdir(dirp)) != NULL) {
		if (dp->d_ino == 0)
			continue;
		if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue;
		if (strlen(name) + 1 + strlen(dp->d_name) >= sizeof(path) - 1) {
			run_err("%s/%s: name too long", name, dp->d_name);
			continue;
		}
		(void) snprintf(path, sizeof path, "%s/%s", name, dp->d_name);
		vect[0] = path;

		source(1, vect, udtscp);
	}
	(void) closedir(dirp);
	(void) atomicio(vwrite, remout, (void *)"E\n", 2);
	if (verbose_mode) fprintf(stderr, "%s%s %s E\n", nodemode, ftitle, atomicioWrite);
	response(ftitle);
}

void
bwlimit(int amount)
{
	static struct timeval bwstart, bwend;
	static int lamt, thresh = 16384;
	u_int64_t waitlen;
	struct timespec ts, rm;

	if (!timerisset(&bwstart)) {
		gettimeofday(&bwstart, NULL);
		return;
	}

	lamt += amount;
	if (lamt < thresh)
		return;

	gettimeofday(&bwend, NULL);
	timersub(&bwend, &bwstart, &bwend);
	if (!timerisset(&bwend))
		return;

	lamt *= 8;
	waitlen = (u_int64_t)((double)1000000L * lamt / limit_rate);

	bwstart.tv_sec = waitlen / 1000000L;
	bwstart.tv_usec = waitlen % 1000000L;

	if (timercmp(&bwstart, &bwend, >)) {
		timersub(&bwstart, &bwend, &bwend);

		/* Adjust the wait time */
		if (bwend.tv_sec) {
			thresh /= 2;
			if (thresh < 2048)
				thresh = 2048;
		} else if (bwend.tv_usec < 10000) {
			thresh *= 2;
			if (thresh > COPY_BUFLEN * 4)
				thresh = COPY_BUFLEN * 4;
		}

		TIMEVAL_TO_TIMESPEC(&bwend, &ts);
		while (nanosleep(&ts, &rm) == -1) {
			if (errno != EINTR)
				break;
			ts = rm;
		}
	}

	lamt = 0;
	gettimeofday(&bwstart, NULL);
}

void
sink(int argc, char **argv, UDTScp& udtscp, int rsw)
{
	const char *ftitle = "[sink]";
	static BUF buffer;
	struct stat stb;
	enum {
		YES, NO, DISPLAYED
	} wrerr;
	BUF *bp;
	off_t i;
	size_t j, count, filesize;
	int amt, exists, first, ofd;
	mode_t mode, omode, mask;
	off_t size, statbytes;
	int setimes, targisdir, wrerrno = 0;
	char ch, *cp, *np, *targ, *vect[1], buf[2048];
	const char *why;
	struct timeval tv[2];

#define	atime	tv[0]
#define	mtime	tv[1]
#define	SCREWUP(str)	{ why = str; goto screwup; }

	setimes = targisdir = 0;
	mask = umask(0);
	if (!pflag)
		(void) umask(mask);
	if (argc != 1) {
		run_err("ambiguous target");
		exit(1);
	}
	targ = *argv;
	if (targetshouldbedirectory)
		verifydir(targ);

// 0.9.14-1 recursive
	if (rsw == 1){
	  (void) atomicio(vwrite, remout, (void *)"", 1);
	  if (verbose_mode) fprintf(stderr, "%s%s %s %s\n",
		nodemode, ftitle, atomicioWrite, null);
	}
	if (stat(targ, &stb) == 0 && S_ISDIR(stb.st_mode))
		targisdir = 1;
	for (first = 1;; first = 0) {
		cp = buf;
		if (atomicio(read, remin, cp, 1) != 1)
			return;
		if (*cp++ == '\n')
			SCREWUP("unexpected <newline>");
		do {
			if (atomicio(read, remin, &ch, sizeof(ch)) != sizeof(ch))
				SCREWUP("lost connection");
			*cp++ = ch;
		} while (cp < &buf[sizeof(buf) - 1] && ch != '\n');
		*cp = 0;
		if (verbose_mode) fprintf(stderr, "%s%s Recieve strings: %s",
			nodemode, ftitle, buf);

		if (buf[0] == '\01' || buf[0] == '\02') {
			if (iamremote == 0)
				(void) atomicio(vwrite, STDERR_FILENO,
					buf + 1, strlen(buf + 1));
				if (verbose_mode) fprintf(stderr, "%s%s %s %s\n",
					nodemode, ftitle, atomicioWrite, buf + 1);
			if (buf[0] == '\02')
				exit(1);
			++errs;
			continue;
		}
		if (buf[0] == 'E') {
			(void) atomicio(vwrite, remout, (void *)"", 1);
			if (verbose_mode) fprintf(stderr, "%s%s %s %s\n",
				nodemode, ftitle, atomicioWrite, null);
			return;
		}
		if (ch == '\n')
			*--cp = 0;

		cp = buf;
		if (*cp == 'T') {
			setimes++;
			cp++;
			mtime.tv_sec = strtol(cp, &cp, 10);
			if (!cp || *cp++ != ' ')
				SCREWUP("mtime.sec not delimited");
			mtime.tv_usec = strtol(cp, &cp, 10);
			if (!cp || *cp++ != ' ')
				SCREWUP("mtime.usec not delimited");
			atime.tv_sec = strtol(cp, &cp, 10);
			if (!cp || *cp++ != ' ')
				SCREWUP("atime.sec not delimited");
			atime.tv_usec = strtol(cp, &cp, 10);
			if (!cp || *cp++ != '\0')
				SCREWUP("atime.usec not delimited");
			(void) atomicio(vwrite, remout, (void *)"", 1);
			if (verbose_mode) fprintf(stderr, "%s%s %s %s\n",
				nodemode, ftitle, atomicioWrite, null);
			continue;
		}

		int port;
		char bufport[1024];
		if (*cp == 'P') {
			cp++;
			port = strtol(cp, NULL, 10);
			sprintf(bufport, "%d", port);
			if (verbose_mode) fprintf(stderr, "%s%s Send port number: %d\n",
				nodemode, ftitle, port);
			(void) atomicio(vwrite, remout, (void *)"", 1);
			if (verbose_mode) fprintf(stderr, "%s%s %s %s\n",
				nodemode, ftitle, atomicioWrite, null);
			continue;
		}

		if (*cp != 'C' && *cp != 'D') {
			/*
			 * Check for the case "rcp remote:foo\* local:bar".
			 * In this case, the line "No match." can be returned
			 * by the shell before the rcp command on the remote is
			 * executed so the ^Aerror_message convention isn't
			 * followed.
			 */
			if (first) {
				run_err("%s", cp);
				exit(1);
			}
			SCREWUP("expected control record");
		}


		mode = 0;
		for (++cp; cp < buf + 5; cp++) {
			if (*cp < '0' || *cp > '7')
				SCREWUP("bad mode");
			mode = (mode << 3) | (*cp - '0');
		}
		if (*cp++ != ' ')
			SCREWUP("mode not delimited");

		for (size = 0; isdigit(*cp);)
			size = size * 10 + (*cp++ - '0');
		if (*cp++ != ' ')
			SCREWUP("size not delimited");
		if ((strchr(cp, '/') != NULL) || (strcmp(cp, "..") == 0)) {
			run_err("error: unexpected filename: %s", cp);
			exit(1);
		}
		filesize = size;

		if (targisdir) {
			static char *namebuf;
			static size_t cursize;
			size_t need;

			need = strlen(targ) + strlen(cp) + 250;
			if (need > cursize) {
				if (namebuf)
					xfree(namebuf);
				namebuf = (char *)xmalloc(need);
				cursize = need;
			}
			(void) snprintf(namebuf, need, "%s%s%s", targ,
			    strcmp(targ, "/") ? "/" : "", cp);
			np = namebuf;
		} else
			np = targ;
		curfile = cp;
		exists = stat(np, &stb) == 0;
		if (buf[0] == 'D') {
			int mod_flag = pflag;
			if (!iamrecursive)
				SCREWUP("received directory without -r");
			if (exists) {
				if (!S_ISDIR(stb.st_mode)) {
					errno = ENOTDIR;
					run_err("%s: %s", np, strerror(errno));
					continue;
//					goto bad;
				}
				if (pflag)
					(void) chmod(np, mode);
			} else {
				/* Handle copying from a read-only
				   directory */
				mod_flag = 1;
				if (mkdir(np, mode | S_IRWXU) < 0){
					run_err("%s: %s", np, strerror(errno));
					continue;
//					goto bad;
				}
			}
			vect[0] = xstrdup(np);
			sink(1, vect, udtscp, 1);
			if (setimes) {
				setimes = 0;
				if (utimes(vect[0], tv) < 0)
					run_err("%s: set times: %s",
					    vect[0], strerror(errno));
			}
			if (mod_flag)
				(void) chmod(vect[0], mode);
			if (vect[0])
				xfree(vect[0]);
			continue;
		}
		omode = mode;
		mode |= S_IWRITE;
// 0.9.13-2 write protected check
		if ((ofd = open(np, O_WRONLY|O_CREAT, mode)) < 0) {
bad:			run_err("%s: %s", np, strerror(errno));
			continue;
		}
		if (udt_mode) {
			close(ofd);
		}

		(void) atomicio(vwrite, remout, (void *)"", 1);
		if (verbose_mode) fprintf(stderr, "%s%s %s %s\n",
			nodemode, ftitle, atomicioWrite, null);
#ifdef SCPXFER
		if (!udt_mode) {
			if ((bp = allocbuf(&buffer, ofd, COPY_BUFLEN)) == NULL) {
				(void) close(ofd);
				continue;
			}
			cp = bp->buf;
		}
#endif
		wrerr = NO;

#ifdef SCPXFER
		if (!udt_mode) {
			statbytes = 0;
			if (showprogress)
				start_progress_meter(curfile, size, &statbytes);
			set_nonblock(remin);

			for (count = i = 0; i < size; i += bp->cnt) {
				amt = bp->cnt;
				if (i + amt > size)
					amt = size - i;
				count += amt;
				do {
				  j = scpio(read, remin, cp, amt, &statbytes);
				  if (j == 0) {
				    run_err("%s", j != EPIPE ? strerror(errno) :
						    "dropped connection");
				    exit(1);
				  }
				  amt -= j;
				  cp += j;
				} while (amt > 0);

				if (count == bp->cnt) {
				  /* Keep reading so we stay sync'd up. */
				  if (wrerr == NO) {
				    if (atomicio(vwrite, ofd, bp->buf, count) != count){
				      wrerr = YES;
				      wrerrno = errno;
				    }
				    if (verbose_mode) fprintf(stderr, "%s%s %s %s\n",
				      nodemode, ftitle, atomicioWrite, bp->buf);
				   }
				   count = 0;
				   cp = bp->buf;
				}
			}
		}
#endif

		if (udt_mode) {
			char nbuf[2048];
			sprintf(nbuf, "%s", np);
			udtscp.recvfile((char *)nbuf, filesize);
			chmod(np, omode);
		}

#ifdef SCPXFER
		if (!udt_mode) {
			unset_nonblock(remin);

			if (showprogress)
				stop_progress_meter();

			if (count != 0 && wrerr == NO &&
			    atomicio(vwrite, ofd, bp->buf, count) != count) {
				wrerr = YES;
				wrerrno = errno;
			}
			if (verbose_mode) fprintf(stderr, "%s%s %s %s\n",
				nodemode, ftitle, atomicioWrite, bp->buf);
			if (wrerr == NO && (!exists || S_ISREG(stb.st_mode)) &&
			    ftruncate(ofd, size) != 0) {
				run_err("%s: truncate: %s", np, strerror(errno));
				wrerr = DISPLAYED;
			}
		}
#endif

		if (pflag) {
			if (exists || omode != mode)
				if (chmod(np, omode)) {
					run_err("%s: set mode: %s",
					    np, strerror(errno));
					wrerr = DISPLAYED;
				}
		} else {
			if (!exists && omode != mode)
				if (chmod(np, omode & ~mask)) {
					run_err("%s: set mode: %s",
					    np, strerror(errno));
					wrerr = DISPLAYED;
				}
		}
#ifdef SCPXFER
		if (!udt_mode) {
			if (close(ofd) == -1) {
				wrerr = YES;
				wrerrno = errno;
			}
		} 
#endif

		response(ftitle);
		if (setimes && wrerr == NO) {
			setimes = 0;
			if (utimes(np, tv) < 0) {
				run_err("%s: set times: %s",
				    np, strerror(errno));
				wrerr = DISPLAYED;
			}
		}
		switch (wrerr) {
		case YES:
			run_err("%s: %s", np, strerror(wrerrno));
			break;
		case NO:
			(void) atomicio(vwrite, remout, (void *)"", 1);
			if (verbose_mode) fprintf(stderr, "%s%s %s %s\n",
				nodemode, ftitle, atomicioWrite, null);
			break;
		case DISPLAYED:
			break;
		}
	}
screwup:
	run_err("protocol error: %s", why);
	exit(1);
}

int
response(const char *rtitle)
{
	const char *ftitle = "[response]";
	char ch, *cp, resp, rbuf[2048];

	if (atomicio(read, remin, &resp, sizeof(resp)) != sizeof(resp))
		lostconn(0);

	if (verbose_mode) fprintf(stderr, "%s%s%s [%d]\n",
		nodemode, rtitle, ftitle, resp);

	cp = rbuf;
	switch (resp) {
	case 0:		/* ok */
		return (0);
	default:
		*cp++ = resp;
		/* FALLTHROUGH */
	case 1:		/* error, followed by error msg */
	case 2:		/* fatal error, "" */
		do {
			if (atomicio(read, remin, &ch, sizeof(ch)) != sizeof(ch))
				lostconn(0);
			*cp++ = ch;
		} while (cp < &rbuf[sizeof(rbuf) - 1] && ch != '\n');

		if (!iamremote){
			(void) atomicio(vwrite, STDERR_FILENO, rbuf, cp - rbuf);
			if (verbose_mode) fprintf(stderr, "%s%s %s %s\n",
				nodemode, ftitle, atomicioWrite, rbuf);
		}
		++errs;
		if (resp == 1)
			return (-1);
		exit(1);
	}
	/* NOTREACHED */
	return(0);
}

int
response_v(const char *rtitle)
{
	const char *ftitle = "[response_v]";
	char ch, *cp, resp, rbuf[2048];

	if (atomicio(read, remin, &resp, sizeof(resp)) != sizeof(resp))
		lostconn(0);

	if (verbose_mode) fprintf(stderr, "%s%s%s [%d]\n",
		nodemode, rtitle, ftitle, resp);

	cp = rbuf;
	switch (resp) {
	case 0:		/* ok */
		hscp_protocol = 0;
		break;
	default:
		*cp++ = resp;
		/* FALLTHROUGH */
	case 1:		/* error, followed by error msg */
	case 2:		/* fatal error, "" */
		do {
			if (atomicio(read, remin, &ch, sizeof(ch)) != sizeof(ch))
				lostconn(0);
			*cp++ = ch;
		} while (cp < &rbuf[sizeof(rbuf) - 1] && ch != '\n');

		hscp_protocol = strtol(rbuf, NULL, 10);
		break;
	}
	/* NOTREACHED */
	if (verbose_mode) fprintf(stderr, "%s%s%s HSCP Protocol = %d\n",
		nodemode, rtitle, ftitle, hscp_protocol);
	return(0);
}

void
usage(void)
{
	title_display();
	(void) fprintf(stderr,
"usage: hscp [-1246BCpqrv] [-c cipher] [-F ssh_config] [-i identity_file]\n"
"            [-l limit] [-o ssh_option] [-P port] [-S program]\n"
"            [-R] [-I display_mode] [-U]\n"
"            [[user@]host1:]file1 ... [[user@]host2:]file2\n");
	exit(1);
}

void
title_display(void)
{
	if (verbose_mode) fprintf(stderr, "%s ", nodemode);
	fprintf(stderr, "HSCP v.%s\n", HSCP_VERSION);
}

/*
void
title_display_server(void)
{
	fprintf(stderr, " server v.%s\n", HSCP_VERSION);
}
*/

void
run_err(const char *fmt,...)
{
	static FILE *fp;
	va_list ap;

	++errs;
	if (fp != NULL || (remout != -1 && (fp = fdopen(remout, "w")))) {
		(void) fprintf(fp, "%c", 0x01);
		(void) fprintf(fp, "hscp: ");
		va_start(ap, fmt);
		(void) vfprintf(fp, fmt, ap);
		va_end(ap);
		(void) fprintf(fp, "\n");
		(void) fflush(fp);
	}

	if (!iamremote) {
		va_start(ap, fmt);
		vfprintf(stderr, fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
}

void
verifydir(char *cp)
{
	struct stat stb;

	if (!stat(cp, &stb)) {
		if (S_ISDIR(stb.st_mode))
			return;
		errno = ENOTDIR;
	}
	run_err("%s: %s", cp, strerror(errno));
	killchild(0);
}

int
okname(char *cp0)
{
	int c;
	char *cp;

	cp = cp0;
	do {
		c = (int)*cp;
		if (c & 0200)
			goto bad;
		if (!isalpha(c) && !isdigit(c)) {
			switch (c) {
			case '\'':
			case '"':
			case '`':
			case ' ':
			case '#':
				goto bad;
			default:
				break;
			}
		}
	} while (*++cp);
	return (1);

bad:	fprintf(stderr, "%s: invalid user name\n", cp0);
	return (0);
}

BUF *
allocbuf(BUF *bp, int fd, int blksize)
{
	size_t size;
#ifdef HAVE_STRUCT_STAT_ST_BLKSIZE
	struct stat stb;

	if (fstat(fd, &stb) < 0) {
		run_err("fstat: %s", strerror(errno));
		return (0);
	}
	size = roundup(stb.st_blksize, blksize);
	if (size == 0)
		size = blksize;
#else /* HAVE_STRUCT_STAT_ST_BLKSIZE */
	size = blksize;
#endif /* HAVE_STRUCT_STAT_ST_BLKSIZE */
	if (bp->cnt >= size)
		return (bp);
	if (bp->buf == NULL)
		bp->buf = (char *)xmalloc(size);
	else
		bp->buf = (char *)xrealloc(bp->buf, 1, size);
	memset(bp->buf, 0, size);
	bp->cnt = size;
	return (bp);
}

void
lostconn(int signo)
{
	if (!iamremote)
		write(STDERR_FILENO, "lost connection\n", 16);
	if (signo)
		_exit(1);
	else
		exit(1);
}
