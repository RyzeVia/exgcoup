/*
 * execext.c
 *
 *  Created on: 2014/05/12
 *      Author: RyzeVia
 */

#include "egc_pops.h"

#define CONNECTPORT 36460
#define HOSTLEN	256
#define MAXBUF	256

#define SN_UNDEF	-1
#define SN_LISTENER	0
#define SN_NEWCONN	1
#define SN_CLIENT	2

int sockkind[NETTOOL_MAXSOCK];
char* cmdform = "ssh -f -R %d:%s:%d %s %s";
char* cserv = "nettool/tt -p 12345";
char* cclie = "echo test | /home/jitumoto/nettool/nettool/tt -p 12345 -h 127.0.0.1";


int main(int argc, char** argv) {

	char buf[MAXBUF], buf2[MAXBUF];
	int pd, rv, done = 0;
	TERM tm;

	PMG_get_terminfo(&tm);

	PMG_invoke_subpg(cserv);
//	printf("invoke %s\n", cserv);

	sprintf(buf, cmdform, 12345, "127.0.0.1", 12345, "hnd.cspp.cc.u-tokyo.ac.jp", cclie);
	printf("invoke %s\n", buf);
	PMG_invoke_subpg_interact(buf, &pd);

	while(!done){
		rv = read(pd, buf, sizeof(buf));
		buf[rv] = '\0';
		if(strstr(buf, ":") != NULL){
			printf("%s", buf);
			fflush(stdout);
			done = 1;
		}
	}

//	printf("Password:");
//	fflush(stdout);
	PMG_disable_termecho(&tm);
	rv = read(STDIN_FILENO, buf2, sizeof(buf2));
	PMG_set_termecho(&tm);
	buf2[rv]='\0';
	write(pd, buf2, sizeof(buf2));

#if 1
	done = 0;
	buf[0] = '\0';
	while(!done){
		rv = read(pd, buf, sizeof(buf));
		buf[rv] = '\0';
		printf("%s\n", buf);
	}
#endif

	return EXIT_SUCCESS;
}
