/*
 * libpmg.h
 *
 *  Created on: 2014/05/12
 *      Author: RyzeVia
 */

#ifndef PROCMNG_H_
#define PROCMNG_H_

#define _XOPEN_SOURCE	600
#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>
#include <fcntl.h>

//terminal control
#include <termios.h>
#include <sys/ioctl.h>

#define PMG_ARG_MAXNUM	(128)
#define PMG_ARG_MAXLEN	(1024)
#define PMG_SPLITTER " "

typedef struct TERM_ {
	struct termio tty;
//	int setted;
} TERM;

pid_t PMG_invoke_subpg(char* cmdline);
pid_t PMG_invoke_subpg_interact(char* cmdline, int *tty_descriptor);
//pid_t PMG_invoke_subpg_interact2(char* cmdline, int *tty_ind, int *tty_outd);
void PMG_get_terminfo(TERM *tm);
void PMG_disable_termecho(TERM *tm);
void PMG_set_termecho(TERM *tm);

#endif /* PROCMNG_H_ */
