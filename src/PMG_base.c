
/*
 * PMG_base.c
 *
 *  Created on: 2014/05/12
 *      Author: RyzeVia
 */
#define GLOBAL_DEFINITION
#include "pmg_sys.h"

pid_t PMG_invoke_subpg(char* cmdline) {
//	char buf[128];
	pid_t fpid;

	fpid = fork();
	if (fpid < 0) {
		ERRORF("fork:");
		ERROR_EXIT();
	}

	if (fpid == 0) {
		char* splitptr[PMG_ARG_MAXNUM];
		char splitbuf[PMG_ARG_MAXLEN];

		SPMG_split_cmdline(cmdline, PMG_SPLITTER, splitptr, splitbuf);

		execvp(splitptr[0], splitptr);
		//ERRORF("%s was finished\n", cmdline);
		ERROR_EXIT();
	}

	return fpid;
}

static int SPMG_get_newpty(int* master_ttyd, int* slave_ttyd) {
	int ttym, ttys;
	int rv;

	ttym = posix_openpt(O_RDWR);
	if (ttym < 0) {
		ERRORF("posix_openpt:");
		return EXIT_FAILURE;
	}

	rv = grantpt(ttym);
	if (rv != 0) {
		ERRORF("grantpr:");
		return EXIT_FAILURE;
	}

	rv = unlockpt(ttym);
	if (rv != 0) {
		ERRORF("unlockpt:");
		return EXIT_FAILURE;
	}

	ttys = open(ptsname(ttym), O_RDWR);
	*master_ttyd = ttym;
	*slave_ttyd = ttys;

	return EXIT_SUCCESS;

}

pid_t PMG_invoke_subpg_interact(char* cmdline, int *tty_descriptor) {
	int rv, ttym =-1, ttys=-1;
	pid_t fpid;

	SPMG_get_newpty(&ttym, &ttys);

	fpid = fork();
	if (fpid == 0) {
		struct termios sts_old;
		struct termios sts_new;

		char* splitptr[PMG_ARG_MAXNUM];
		char splitbuf[PMG_ARG_MAXLEN];

		close(ttym);

		rv = tcgetattr(ttys, &sts_old);
		sts_new = sts_old;
		cfmakeraw(&sts_new);
		tcsetattr(ttys, TCSANOW, &sts_new);

		close(0); // Close standard input (current terminal)
		close(1); // Close standard output (current terminal)
		close(2); // Close standard error (current terminal)

		dup(ttys); // PTY becomes standard input (0)
		dup(ttys); // PTY becomes standard output (1)
		dup(ttys); // PTY becomes standard error (2)

		/*
		 dup2(fileno(stdout), ttys);
		 dup2(fileno(stdin), ttys);
		 dup2(fileno(stderr), ttys);

		 close(fileno(stdout));
		 close(fileno(stdin));
		 close(fileno(stderr));
		 */
		close(ttys);
		setsid();
		ioctl(0, TIOCSCTTY, 1);

		SPMG_split_cmdline(cmdline, PMG_SPLITTER, splitptr, splitbuf);
		execvp(splitptr[0], splitptr);
		assert(0);
	}

	*tty_descriptor = ttym;
	return fpid;
}

/*
pid_t PMG_invoke_subpg_interact2(char* cmdline, int *tty_ind, int *tty_outd) {
	int rv, ttymin, ttymout, ttysin, ttysout;
	pid_t fpid;

	SPMG_get_newpty(&ttymin, &ttysin);
	SPMG_get_newpty(&ttymout, &ttysout);

	fpid = fork();
	if (fpid < 0) {
		ERRORF("fork:");
		ERROR_EXIT();
	}

	if (fpid == 0) {
		struct termios sts_old;
		struct termios sts_new;

		char* splitptr[PMG_ARG_MAXNUM];
		char splitbuf[PMG_ARG_MAXLEN];

		close(ttymin);
		close(ttymout);

		rv = tcgetattr(ttysin, &sts_old);
		sts_new = sts_old;
		cfmakeraw(&sts_new);
		tcsetattr(ttysin, TCSANOW, &sts_new);

		rv = tcgetattr(ttysout, &sts_old);
		sts_new = sts_old;
		cfmakeraw(&sts_new);
		tcsetattr(ttysout, TCSANOW, &sts_new);

		close(0); // Close standard input (current terminal)
		close(1); // Close standard output (current terminal)
		close(2); // Close standard error (current terminal)

		dup(ttysin); // PTY becomes standard input (0)
		dup(ttysout); // PTY becomes standard output (1)
		dup(ttysout); // PTY becomes standard error (2)

		close(ttysin);
		close(ttysout);
		setsid();
		ioctl(0, TIOCSCTTY, 1);

		SPMG_split_cmdline(cmdline, PMG_SPLITTER, splitptr, splitbuf);
		execvp(splitptr[0], splitptr);
		assert(0);
	}

	*tty_ind = ttymin;
	*tty_outd = ttymout;
	return fpid;
}
*/
/* HINT: SPMG_split_cmdline
 * After called this routine, the variable splitbuf become undefined.
 */
static void SPMG_split_cmdline(char* cmdline, char* sep, char** splitptr,
		char* splitbuf) {
	int id = 0;
	char* tok;

	strncpy(splitbuf, cmdline, PMG_ARG_MAXLEN);

	for (tok = strtok(splitbuf, sep); (id < PMG_ARG_MAXNUM) && (tok != NULL);
			tok = strtok(NULL, sep), id++) {
		splitptr[id] = tok;
	}

	splitptr[id] = NULL;

	return;
}

void PMG_get_terminfo(TERM *tm){
	ioctl(0, TCGETA, &(tm->tty));
}

void PMG_disable_termecho(TERM *tm) {
	struct termio tty;
	tty = tm->tty;

	tty.c_lflag &= ~ECHO;
	tty.c_lflag |= ECHONL;

	ioctl(0, TCSETAF, &tty);

}

void PMG_set_termecho(TERM *tm) {
	struct termio tty;
	tty = tm->tty;

	ioctl(0, TCSETAF, &tty);
}


