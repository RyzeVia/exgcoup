/*
 * EGC_base.c
 *
 *  Created on: 2014/05/14
 *      Author: RyzeVia
 */


#include "egc_sys.h"
#include <getopt.h>

static void print_help();
static void start_serv();
static void start_serv_and_connect(int lport, int cport, int hostid);
char* PEGC_get_stdin(char* prompt, char* buf, int bufsize, int echoflag);
static void SPEGC_serv_job_submit(int i);

int main(int argc, char** argv){
	/* set default value */
	char* filename = DEF_EGC_CONFIG;
	char* pwd = NULL;
	char hn[MAX_EGC_HOSTLEN];
	int connect_port = -1;

	/* option decoding */
	int oint;
	int oind;
	static struct option long_options[] = {
			{"help", no_argument, NULL, 'h'},
			{"configure", required_argument, NULL, 'c'},
			{"connect_port", required_argument, NULL, 'p'},
			{"listener_port", required_argument, NULL, 'l'},
			{"hostid", required_argument, NULL, 'i'},
			{"dir", required_argument, NULL, 'd'},
			{0, 0, 0, 0}
	};
	static char *opts ="hc:p:l:i:d:";
	while((oint = getopt_long(argc, argv, opts, long_options, &oind)) != -1){
		switch(oint){
		case 'h':
			print_help();
			exit(EXIT_SUCCESS);
			break;
		case 'c':
			filename = optarg;
			setenv("EGC_CONFIG", filename, 1);
			break;
		case 'p':
			connect_port = atoi(optarg);
			break;
		case 'l':
			listener_port = atoi(optarg);
			break;
		case 'i':
			setenv("EGC_JID", optarg, 1);
			hostid = atoi(optarg);
			break;
		case 'd':
			pwd = optarg;
			chdir(pwd);
			break;
		default:
			ERROR_LOCF("ERR: An unknown option is sppointed.\n");
			print_help();
			exit(EXIT_FAILURE);
		}

	}

	/* read config on global value*/
	filename = CFR_getvalue("EGC_CONFIG", DEF_EGC_CONFIG);
	CFR_setenv_from_file(filename, FMT_EGC_CONFIG);
	cmdname = argv[0];

	gethostname(hn, MAX_EGC_HOSTLEN);
	DEBUGF(DEBUG_LEVEL_INFO, "INFO: egcpops start on %s\n", hn);

//#ifdef DEBUG_ENABLE
//	PMG_invoke_subpg("sample/netstat.sh");
//#endif

	PEGC_comm_init();
//	PEGC_event_init();

	if(connect_port < 0){
		start_serv(listener_port);
	}else{
		start_serv_and_connect(listener_port, connect_port, hostid);
	}
	return EXIT_SUCCESS;
}

static void print_help(){
	printf("Usage: EXGCOUP Pop server\n");
	printf("    -h, --help     : showing this help\n");
	printf("    -c, --configure: configure file name\n");
}


int PEGC_serv_exec_on_remotehost(int i, int listener_port, p_commhost *host){
	char* hostname;
	int bufsize, port, sshport;
	char wsbuf[256];
	char prompt[256];
	char cmdline[256];
	char path[MAXPATHLEN];
	char* filename;
	char* config;
	char* cmd;
//	char* pwd;

	//Read Config
	filename = CFR_getvalue("EGC_CONFIG", DEF_EGC_CONFIG);
	if(!getcwd(path, sizeof(path))){
		ERROR_LOCF("Cannot get pwd\n");
		ERROR_EXIT();
	}

	sprintf(wsbuf, "EGC_HOSTNAME%d", i);
	hostname = getenv(wsbuf);
	if(NULL == (hostname = CFR_getvalue(wsbuf, NULL))){
		ERRORF("Config: %s is not defined.", wsbuf);
		ERROR_EXIT();
	}

	sprintf(wsbuf, "EGC_HOSTBUFSIZE%d", i);
	bufsize = CFR_getvalue_byint(wsbuf, DEF_EGC_PACKET_SIZE);
	if(host->hdr == NULL){
		host->hdr = PEGC_comm_newhdr(EGC_COMM_MODE_SOCKET, bufsize, DEF_EGC_INTERBUF_TOSEC);
	}

	sprintf(wsbuf, "EGC_PROXYPORT%d", i);
	port = CFR_getvalue_byint(wsbuf, DEF_EGC_PXYPORT);

	sprintf(wsbuf, "EGC_SSHPORT%d", i);
	sshport = CFR_getvalue_byint(wsbuf, DEF_EGC_SSHPORT);

	memcpy(host->hdr->addr.name, hostname, MAX_EGC_HOSTLEN);
	host->hdr->addr.portnum = sshport;

	sprintf(prompt, "[%s] User Name:", hostname);
	if(host->uid[0] == 0){
		PEGC_get_stdin(prompt, host->uid, MAX_EGC_UIDLEN, EGC_INPUT_TERMON);
	}

	sprintf(prompt, "[%s] Password or Pass phrase:", hostname);
	if(host->pass[0] == 0){
		PEGC_get_stdin(prompt, host->pass, MAX_EGC_PASSLEN, EGC_INPUT_TERMOFF);
	}

	if(hostid != i){
		cmd = PEGCU_get_confwithid("EGC_POPSERV_PATH", i);
		DEBUGF(DEBUG_LEVEL_INFO, "INFO: egcpops path %s\n", cmd);
		if(cmd == NULL){
			//PATH が通ってる前提
			cmd = cmdname;
		}
		config = PEGCU_get_confwithid("EGC_CONFIG", i);
		if(config == NULL){
			config = filename;
		}
//		sprintf(cmdline, FMT_EGC_CMDEXEC, path, cmdname, port, i, filename);
		DEBUGF(DEBUG_LEVEL_INFO, "INFO: egcpops path %s\n", cmd);
		sprintf(cmdline, FMT_EGC_CMDEXEC, cmd, port, i, config);
		DEBUGF(DEBUG_LEVEL_INFO, "INFO: SSH invoke cmd=%s\n", cmdline);
		PEGCU_invoke_sshtunnel_R(hostname, sshport, port, listener_port, cmdline, host->uid, host->pass);
	}
	return EXIT_SUCCESS;

}

char* PEGC_get_stdin(char* prompt, char* buf, int bufsize, int echoflag){
	struct termios termconf, newconf;
	char* rv;

	if(echoflag == EGC_INPUT_TERMOFF){
		tcgetattr(0, &termconf);
		newconf = termconf;
		newconf.c_lflag &= (~ECHO);
		tcsetattr(0, TCSANOW, &newconf);
	}

	memset(buf, 0, bufsize);
	printf("%s", prompt);
	rv = fgets(buf, bufsize, stdin);
	sscanf(buf, "%[^\n]", buf);
	if(rv == NULL){
		ERROR_LOCF("Input Error: too long input (limit=%d)\n", bufsize-1);
		ERROR_EXIT();
	}

	if(echoflag == EGC_INPUT_TERMOFF){
		tcsetattr(0, TCSANOW, &termconf);
		printf("\n");
	}

	return rv;
}

static void start_serv(int lport){
	int nhost;
	int i;
	p_commhdr *lhdr, *llhdr;
	p_eventhdr *ehdr;
	char wsbuf[MAX_EGC_KEYVALUE_SIZE];
	char* lmode;
	struct timeval intra_tv = {DEF_EGC_INTRABUF_TOSEC, 0};
//	struct timeval inter_tv = {DEF_EGC_INTERBUF_TOSEC, 0};
	EGC_PRE_DBG("SERVER");

//	event_enable_debug_mode();
	/* event base の設定 */
	ehdr = PEGC_event_newhdr();

	/* listener event を作成し、登録 */
	lhdr = PEGC_comm_newhdr_blank(EGC_COMM_MODE_SOCKET);
	lhdr->addr.name[0] = '\0';
	lhdr->addr.portnum = lport;
	lhdr->call->listen(lhdr);
	PEGC_event_register(ehdr, lhdr, EV_READ, PEGC_intercomm_cblistener);

	/* SSH 路をつくる */
	if((nhost = CFR_getvalue_byint("EGC_HOSTNUM", 0)) <= 0){
		ERROR_LOCF("Config: EGC_HOSTNUM is unacceptable. it need >=1.\n");
		ERROR_EXIT();
	}
	if(nhost > MAX_EGC_HOSTNUM){
		ERROR_LOCF("Config: EGC_HOSTNUM is too many. it need < %d.\n", MAX_EGC_HOSTNUM);
		ERROR_EXIT();
	}

	/* このサーバのサーバネームとUID、PASSWORDを作成 （いらない？） */
//	PEGC_get_stdin("UID on -This server-:", localhost.uid, MAX_EGC_UIDLEN, EGC_INPUT_TERMON);
//	PEGC_get_stdin("Password or Pass phrase on -This server-:", localhost.pass, MAX_EGC_UIDLEN, EGC_INPUT_TERMOFF);


	CALLOC(p_commhost*, hosts, nhost, sizeof(p_commhost));
	for(i = 0; i < nhost; i++){
		PEGC_serv_exec_on_remotehost(i, lport, &(hosts[i]));
	}

	/* Intra connection の リスナ生成 */
	sprintf(wsbuf, "EGC_INTRA_MODE%d", hostid);
	lmode = CFR_getvalue(wsbuf, DEF_EGC_INTRALMODE);
	if(strncmp(lmode, "file", 4) == 0){
		llhdr = PEGC_comm_newhdr_blank(EGC_COMM_MODE_FILE);
		// アドレスセットしてリスナ作成へ
		//FIXME: 初期値の与え方はsocket/file で違うが、このレイヤでは統一的に扱えるべき
		sprintf(wsbuf, "EGC_INTRA_HOST_LISTENPATH%d", hostid);
		llhdr->addr.name[0] = '\0';
		llhdr->addr.portfile = CFR_getvalue(wsbuf, NULL);
		DEBUGF(DEBUG_LEVEL_INFO, "INFO: listener file candidate is: %s%s\n", llhdr->addr.name, llhdr->addr.portfile);

		if(llhdr->addr.portfile == NULL){
			ERRORF("Config: EGC_INTRA_HOST_LISTENPATH%d is not defined.\n", hostid);
			ERROR_EXIT();
		}
		PEGC_comm_set_bursterror_timer(llhdr, &intra_tv);
		llhdr->call->listen(llhdr);
		PEGC_event_register(ehdr, llhdr, EV_READ, PEGC_intracomm_cblistener);

	}else if(strncmp(lmode, "socket", 6) == 0){
		llhdr = PEGC_comm_newhdr_blank(EGC_COMM_MODE_SOCKET);
		assert(0);
	}else{
		ERRORF("Intracomm mode is not expected: %s\n", lmode);
		ERROR_EXIT();
	}

	SPEGC_serv_job_submit(hostid);

	/* イベントをフックを回す */
	PEGC_event_loop(ehdr);

	/* 終了 */
	PEGC_event_freehdr(ehdr);


	EGC_POST_DBG("SERVER");
}

static void SPEGC_serv_job_submit(int i){
	char *cmdline;
	char buf[MAX_EGC_KEYVALUE_SIZE];


	sprintf(buf, "EGC_JOBSUBMIT%d", i);
	if(NULL == (cmdline = CFR_getvalue(buf, NULL))){
		ERRORF("No submitting cmd for Host %d\n", i);
		ERROR_EXIT();
	}

	printf("JOBSUBMITTING by %s\n", buf);

	PMG_invoke_subpg(cmdline);

}


static void start_serv_and_connect(int lport, int cport, int hostid){
	//EXAM: 三拠点送受信と成る場合は　start_serv との融合を行う
	int bufsize;
	p_commhdr *lhdr, *chdr;// *llhdr;
	p_eventhdr *ehdr;//, *lehdr;
//	char wsbuf[256];
//	char* host0name;
//	char* lmode;
//	char* ctmp;
//	struct timeval intra_tv = {DEF_EGC_INTRABUF_TOSEC, 0};
	EGC_PRE_DBG("SERVER");

	/* event base の設定 */
	ehdr = PEGC_event_newhdr();

	/* listener event を作成し、登録 */
	lhdr = PEGC_comm_newhdr_blank(EGC_COMM_MODE_SOCKET);
	lhdr->addr.name[0] = '\0';
	lhdr->addr.portnum = lport;
	lhdr->call->listen(lhdr);
	PEGC_event_register(ehdr, lhdr, EV_READ, PEGC_intercomm_cblistener);

	/* connect する */
	// host 0 を設定ファイルより摘出してコネクトかける
	// EXAM: 接続先を複数取る場合にここを書き換える
	chdr = PEGC_comm_newhdr_blank(EGC_COMM_MODE_SOCKET);
	/*
	if(NULL == (host0name = CFR_getvalue("EGC_HOSTNAME0", NULL))){
		ERROR_LOCF("Config: host0 is not defined.\n");
		ERROR_EXIT();
	}
	*/
	strncpy(chdr->addr.name, DEF_LOCALHOST_IP, MAX_EGC_HOSTLEN);
	chdr->addr.portnum = cport;

	DEBUGF(DEBUG_LEVEL_INFO, "INFO: connect to %s and port %d\n", chdr->addr.name, chdr->addr.portnum);

	bufsize = CFR_getvalue_byint("EGC_PACKET_SIZE", DEF_EGC_PACKET_SIZE);
	chdr->mbuf = chdr->call->bufalloc(bufsize);
	chdr->call->connect(chdr);

//  BREAD(chdr->fd, chdr->mbuf->buf, MAX_EGC_GREETING_PKT);
//	DEBUGF(DEBUG_LEVEL_INFO, "INFO: wait greeting msg on %d\n", chdr->fd);
//	read(chdr->fd, chdr->mbuf->buf, MAX_EGC_GREETING_PKT);
//	DEBUGF(DEBUG_LEVEL_INFO, "%s\n", chdr->mbuf->buf);

	PEGC_event_register(ehdr, chdr, EV_READ, PEGC_intercomm_cbdata);

	SPEGC_serv_job_submit(hostid);


	/* イベントフックを回す*/
	PEGC_event_loop(ehdr);

	/* 終了 */
	PEGC_event_freehdr(ehdr);

}

