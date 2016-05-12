/*
 * EGC_lib.c
 *
 *  Created on: 2014/10/10
 *      Author: RyzeVia
 */
#define GLOBAL_DEFINITION
#define DEBUG_ENABLE

#include "egc_sys.h"

int id = -1, jid = -1;
EGC_COMM_MODE mode = -1;
int pktsize, ctnsize;
char* trashbuf;
char* config;
p_commhdr *dhdr = NULL;

int EGC_init(int uid){
	id = uid;
	EGC_PRE_DBG("INTERNAL");

	jid = PEGCU_get_jobid();
	DEBUGF(DEBUG_LEVEL_INFO, "INFO:[id:%d] jobid:%d\n", id, jid);

	config = PEGCU_get_confpath();
	DEBUGF(DEBUG_LEVEL_INFO, "INFO:[id:%d] Config file:%s\n", id, config);

	CFR_setenv_from_file(config, FMT_EGC_CONFIG);

	PEGC_comm_init();

	mode = PEGC_intracomm_getmode(jid);
	pktsize = CFR_getvalue_byint("EGC_PACKET_SIZE", DEF_EGC_PACKET_SIZE);
	ctnsize = pktsize-sizeof(p_msginfo);
	MALLOC(char*, trashbuf, pktsize);

	EGC_POST_DBG("INTERNAL");
	return EXIT_SUCCESS;
}


int EGC_send(int dest, char* buf, int msgsize){
	p_commhdr hdr;
	static p_commhdr *dhdr;
	static int fd = 0;
	p_msgbuf mbuf;
	char* ge;
	char wsbuf[MAX_EGC_KEYVALUE_SIZE];

	//dhdr = PEGC_comm_gethdr_w_uid(id, dest, &intracomm);
	if(dhdr == NULL){
		dhdr = PEGC_comm_newhdr_blank(mode);
		dhdr->minfo.srcid = id;
		dhdr->minfo.destid = dest;
		dhdr->minfo.messageid = 0;
		dhdr->minfo.flags = 0;
		dhdr->minfo.nbytes = msgsize;
		switch (mode){
		case EGC_COMM_MODE_FILE:

//			if(fd == 0){
				dhdr->mbuf = dhdr->call->bufalloc(pktsize);
				sprintf(wsbuf, "EGC_INTRA_HOST_DATAPATH%d", jid);
				ge = CFR_getvalue(wsbuf, NULL);
				sprintf(dhdr->addr.name, FMT_EGC_INTRS, ge, id);
				sprintf(wsbuf, "EGC_INTRA_HOST_LISTENPATH%d", jid);
				dhdr->addr.portfile=CFR_getvalue(wsbuf, NULL);
				dhdr->call->connect(dhdr);
				fd = dhdr->fd;
//			}
			//ONGOING(0):

			break;
		case EGC_COMM_MODE_SOCKET:
			ERRORF("not implemented\n");
			assert(0);
			break;
		default:
			ERRORF("EGC: unexpected intra comm mode\n");
			ERROR_EXIT();
		}


		PEGC_comm_addhdrchain(dhdr, &intracomm);
	}

	// パケット分割する
	memcpy(&hdr, dhdr, sizeof(p_commhdr));
	hdr.mbuf = &mbuf;

	DEBUGF(DEBUG_LEVEL_INFO, "INFO: minfo=(src:%d, dest:%d, size:%d, id:%d)\n", hdr.minfo.srcid, hdr.minfo.destid, hdr.minfo.nbytes, hdr.minfo.messageid);
	for(mbuf.buf = buf; mbuf.buf < buf+msgsize; mbuf.buf += ctnsize){
		DEBUGF(DEBUG_LEVEL_INFO, "write buf:%p, remain:%d\n", mbuf.buf, hdr.minfo.nbytes);
		dhdr->call->intra_write(dhdr, &hdr);
		hdr.minfo.nbytes -= ctnsize;
	}

	dhdr->minfo.messageid++;

	return msgsize;

}


int EGC_recv(int src, char* buf, int bufsize){
	p_commhdr *shdr;
	p_msginfo head;
//	char* ge;
	char* pbuf;
	char* minfo = (char*)&head;
//	char wsbuf[MAX_EGC_KEYVALUE_SIZE];

	int psize = bufsize, tsize = 0, rv;
	shdr = PEGC_comm_gethdr_w_uid(src, id, &intracomm);
	if(shdr == NULL){
		shdr = PEGC_comm_newhdr_blank(mode);
		shdr->minfo.srcid = src;
		shdr->minfo.destid = id;
		shdr->minfo.messageid = 0;
		shdr->minfo.flags = 0;
		shdr->minfo.nbytes = 0;

		switch (mode){
		case EGC_COMM_MODE_FILE:
			shdr->mbuf = shdr->call->bufalloc(pktsize);
//			sprintf(wsbuf, "EGC_INTRA_HOST_DATAPATH%d", jid);
//			ge = PEGCU_get_confwithid("EGC_INTRA_HOST_DATAPATH", EGC_MYJOBID);
//			sprintf(shdr->addr.name, FMT_EGC_INTRSD, ge, src, id);
//			shdr->addr.portfile=PEGCU_get_confwithid("EGC_INTRA_HOST_DATAPATH", EGC_MYJOBID);
			shdr->addr.portfile=NULL;
			shdr->call->adhoc_open_minfo(shdr);

			//ONGOING(0):ここから読み出しチェックの続行

			break;
		case EGC_COMM_MODE_SOCKET:
			ERRORF("not implemented\n");
			assert(0);
			break;
		default:
			ERRORF("EGC: unexpected intra comm mode\n");
			ERROR_EXIT();
		}
		PEGC_comm_addhdrchain(shdr, &intracomm);

	}

	DEBUGF(DEBUG_LEVEL_INFO, "INFO: expected data size %d\n", psize);
	for(pbuf = buf; psize > 0; pbuf += ctnsize){
		DEBUGF(DEBUG_LEVEL_INFO, "INFO: expected data size %d\n", psize);
//		lseek(shdr->fd, shdr->minfo.nbytes, SEEK_SET);
		rv = BREAD(shdr->fd, minfo, sizeof(p_msginfo));
		if(head.nbytes > bufsize){
			ERRORF("EGC_recv: buffer is too small. it needs %d bytes\n", head.nbytes);
			ERROR_EXIT();
		}
//		lseek(shdr->fd, shdr->minfo.nbytes+sizeof(p_msginfo), SEEK_SET);
//		test code
/*
		int i;
		for(i = 0; i<5; i++){
			fprintf(stderr, "%d\n", ((int*)minfo)[i]);
		}
*/
		DEBUGF(DEBUG_LEVEL_INFO, "INFO:Read buf(%p) to %p\n", buf, pbuf);
		if(head.nbytes >= ctnsize){
			rv = BREAD(shdr->fd, pbuf, ctnsize);
		}else{
			rv = BREAD(shdr->fd, pbuf, head.nbytes);
			rv = BREAD(shdr->fd, trashbuf, ctnsize-head.nbytes);
			break;
		}

//		test code
//		int i;
//		for(i = 0; i< ctnsize/sizeof(int); i++){
//			fprintf(stderr, "%d\n", ((int*)pbuf)[i]);
//		}


		tsize += head.nbytes;
		psize -= ctnsize;
	}
	shdr->minfo.nbytes += psize;
	return tsize;
}

