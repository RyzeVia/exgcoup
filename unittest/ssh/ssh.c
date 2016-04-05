/*
 * ssh.c
 *
 *  Created on: 2015/03/03
 *      Author: RyzeVia
 */


#define GLOBAL_DEFINITION
#include "egc_sys.h"
#include <getopt.h>

char* cserv = "../nettool/tt -p 12345";
char* cclie = "echo test | /home/jitumoto/nettool/nettool/tt -p 12345 -h 127.0.0.1";
char* cclie2 = "echo test | /home/jitumoto/nettool/nettool/tt -p 12346 -h 127.0.0.1";

static void print_help();
static void serv();
//static void clie();

int main(int argc, char** argv){
	/* set default value */
	char* dest = NULL;
	int isserv = 0;
	int port;

	/* option decoding */
	int oint;
	int oind;

	static struct option long_options[] = {
			{"help", no_argument, NULL, 'h'},
			{"port", required_argument, NULL, 'p'},
			{"dest", required_argument, NULL, 'd'},
			{0, 0, 0, 0}
	};
	static char *opts ="hp:d:";
	while((oint = getopt_long(argc, argv, opts, long_options, &oind)) != -1){
		switch(oint){
		case 'h':
			print_help();
			exit(EXIT_SUCCESS);
			break;
		case 'p':
			sscanf(optarg, "%d", &port);
			break;
		case 'd':
			dest = optarg;
			isserv = 1;
			break;
		default:
			ERROR_LOCF("ERR: An unknown option is appointed.\n");
			print_help();
			exit(EXIT_FAILURE);
		}

	}

	serv(argv, argc);

	return EXIT_SUCCESS;
}

static void print_help(){
	printf("Usage: EXGCOUP SSHMG Test\n");
	printf("    -h, --help     : showing this help\n");
	printf("    -p, --port     : using port by socket\n");
	printf("    -d, --dest     : destination host \n");
}

static void serv(int argv, char** argc){
//	int nhost;
	char cmd[256], buf[256];
	int done = 0, rv, pd, pd2;
	EGC_PRE_DBG("SERVER");

	/* SSH ˜H‚ð‚Â‚­‚é */

	PMG_invoke_subpg(cserv);
	printf("invoke %s\n", cserv);
	pd = PEGCU_invoke_sshtunnel_R("ibr.cspp.cc.u-tokyo.ac.jp", 12345, 12345, cclie, "Ryze_Via");

#if 1
	done = 0;
	buf[0] = '\0';
	while(!done){
		rv = read(pd, buf, sizeof(buf));
		buf[rv] = '\0';
		printf("%s:%d\n", buf, rv);
		if(rv == 0){done = 1;}
	}
#endif

	pd2 = PEGCU_invoke_sshtunnel_R("ibr.cspp.cc.u-tokyo.ac.jp", 12346, 12345, cclie2, "Ryze_Via");
#if 1
	done = 0;
	buf[0] = '\0';
	while(!done){
		rv = read(pd2, buf, sizeof(buf));
		buf[rv] = '\0';
		printf("%s\n", buf);
	}
#endif

	EGC_POST_DBG("SERVER");
}
