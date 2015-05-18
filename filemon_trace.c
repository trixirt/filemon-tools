/*
 * Copyright (c) 2013-2014, Juniper Networks, Inc.
 * All rights reserved.
 *
 * You may distribute under the terms of any of:
 *
 * the BSD 2-Clause license
 *
 * Any patches released for this software are to be released under these
 * same license terms.
 *
 * BSD 2-Clause license:
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#ifdef FreeBSD
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <err.h>
#endif
#include "filemon.h"

int cmdline_argc;
char **cmdline_argv;

char log_name[] = "fmtrace.log";
int fd;
int fm_log;

static int run_cmd()
{
	int ret = 1;
	pid_t try_child;
	try_child = fork();
	if (try_child == 0) {
		if (execvp(cmdline_argv[0], cmdline_argv)) {
			printf("Could not execute %s\n", cmdline_argv[0]);
			_exit(-1);
		}
		/* Not expecting to reach here.. */
	} else {
		int status;
		pid_t wait_child;
		wait_child = waitpid(try_child, &status, 0);
	}
	return 0;
}

static int do_filemon_trace()
{
	int ret = 1;
	fd = open("/dev/filemon", O_RDWR);
	if (0 > fd) {
		perror("open failed ");
	} else {
		fm_log = open(log_name,  O_WRONLY |  O_CREAT | O_TRUNC, 0666);
		if (0 > fm_log) {
			perror("open failed ");
		} else {
			if (0 < ioctl(fd, FILEMON_SET_FD, &fm_log)) {
				perror("ioctl FILEMON_SET_FD failed ");
			} else {
				/* Set up these two fd's to close on exec */
				if (0 > fcntl(fd, F_SETFD, FD_CLOEXEC)) {
					perror("fnctl fd FD_CLOEXEC ");
				} else {
					if (0 > fcntl(fm_log, F_SETFD, FD_CLOEXEC)) {
						perror("fnctl log FD_CLOEXEC ");
					} else {
						pid_t pid = getpid();
						if (0 > pid) {
							perror("getpid ");
						} else {
							if (0 > ioctl(fd, FILEMON_SET_PID, &pid))
								perror("ioctl FILEMON_SET_PID failed ");
							else
								ret = run_cmd();
						}
					}
				}
			}
			close(fm_log);
		}
		close(fd);
	}
	return ret;
}
int main(int argc, char *argv[])
{
	int ret = 1;
	if (argc > 1) {
		cmdline_argc = argc - 1;
		cmdline_argv = (char **) malloc((cmdline_argc + 1) * sizeof(char *));
		if (cmdline_argv) {
			int a;
			for (a = 0; a < cmdline_argc; a++) {
				cmdline_argv[a] = strdup(argv[a + 1]);
				if (cmdline_argv[a] == NULL) {
					printf("cmdline_argv %d is null\n", a);
					exit(-1);
				}
			}
			/* NULL terminate the last entry */
			cmdline_argv[cmdline_argc] = NULL;
			ret = do_filemon_trace();
			for (a = 0; a < cmdline_argc; a++)
				free(cmdline_argv[a]);
			free(cmdline_argv);
		}

	}

	return ret;
}
