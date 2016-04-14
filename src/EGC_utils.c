/*
 * EGC_sshmg.c
 *
 *  Created on: 2014/05/14
 *      Author: RyzeVia
 */

#include "egc_sys.h"

#define MAX_COMLINE	(1024)
#define LOCALHOST	"127.0.0.1"
char* cmdform = "ssh %s %d:%s:%d -l %s %s -p %d %s";


int PEGCU_invoke_sshtunnel_R(char* hostname, int sshport, int beyondport, int hereport, char* cmdline, char* uid, char* pass){
	char cmd[MAX_COMLINE], buf[MAX_COMLINE], passin[MAX_COMLINE];
	int sshdsc, rv;
	int done = 0;
	pid_t pid;

	sprintf(cmd, cmdform, "-R", beyondport,  LOCALHOST, hereport, uid, hostname, sshport, cmdline);
	DEBUGF(DEBUG_LEVEL_INFO, "INFO: SSH command line:%s\n", cmd);

	pid = PMG_invoke_subpg_interact(cmd, &sshdsc);
	// Read SSH output until password: or passphrase:
	while(!done){
		rv = read(sshdsc, buf, sizeof(buf));
		buf[rv] = '\0';
		if(strstr(buf, ":") != NULL){
			printf("%s\n", buf);
			fflush(stdout);
			done = 1;
		}
	}
	sprintf(passin, "%s\n", pass);
	rv = write(sshdsc, passin, strlen(passin));

#ifdef DEBUG_SSH_OUTPUT
	pid = fork();
	if(pid == 0){
		done=0;
		while(!done){
			rv = read(sshdsc, buf, sizeof(buf));
			buf[rv] = '\0';
			printf("%s\n", buf);
			fflush(stdout);
		}
	}
#endif
	return sshdsc;
}

int PEGCU_invoke_sshtunnel_L(char* hostname, char* proxyname, int sshport, int beyondport, int hereport, char* cmdline, char* uid, char* pass){
	char cmd[MAX_COMLINE], buf[MAX_COMLINE];
	int sshdsc, rv, done = 0;
	pid_t pid;

	if(proxyname == NULL){
		sprintf(cmd, cmdform, "-L", beyondport, hostname, hereport, uid, hostname, sshport, cmdline);
	}else{
		sprintf(cmd, cmdform, "-L", beyondport, hostname, hereport, uid, proxyname, sshport, cmdline);
	}

	DEBUGF(DEBUG_LEVEL_INFO, "EGC_invoke: cmd=%s\n", cmd);
	pid = PMG_invoke_subpg_interact(cmd, &sshdsc);
	// Read SSH output until password: or passphrase:
	while(!done){
		rv = read(sshdsc, buf, sizeof(buf));
		buf[rv] = '\0';
		if(strstr(buf, ":") != NULL){
//			printf("%s", buf);
//			fflush(stdout);
			done = 1;
		}
	}
	rv = write(sshdsc, pass, sizeof(pass));

	return sshdsc;
}

int PEGCU_get_jobid(){
	char* idstr;
	int jid;
	idstr = getenv("EGC_JID");
	if(idstr == NULL){
		ERRORF("EGC: No Assined JOBID.\n");
		ERRORF("   : the jobid must be set on ENV value \"EGC_JID\".\n");
		ERROR_EXIT();
	}
	jid = atoi(idstr);
	return jid;
}

char* PEGCU_get_confpath(){
	char* config;
	config = getenv("EGC_CONFIG");
	if(config == NULL){
		ERRORF("EGC: No config file.\n");
		ERRORF("   : the file name must be set on ENV value \"EGC_CONFIG\".\n");
		ERROR_EXIT();
	}
	return config;
}

char* PEGCU_get_confwithid(char* token, int id){
	char wsbuf[MAX_EGC_KEYVALUE_SIZE];
	char* rv;
	if(id == EGC_MYJOBID){
		id = PEGCU_get_jobid();
	}
	sprintf(wsbuf, "%s%d", token, id);
	rv = CFR_getvalue(wsbuf, NULL);
	return rv;
}

void PEGCU_htonl_minfo(p_msginfo* newinfo, p_msginfo* info){
	newinfo->destid = htonl(info->destid);
	newinfo->srcid = htonl(info->srcid);
	newinfo->nbytes = htonl(info->nbytes);
	newinfo->messageid = htonl(info->messageid);
	newinfo->flags = htonl(info->flags);
}

void PEGCU_ntohl_minfo(p_msginfo* newinfo, p_msginfo* info){
	newinfo->destid = ntohl(info->destid);
	newinfo->srcid = ntohl(info->srcid);
	newinfo->nbytes = ntohl(info->nbytes);
	newinfo->messageid = ntohl(info->messageid);
	newinfo->flags = ntohl(info->flags);
}
