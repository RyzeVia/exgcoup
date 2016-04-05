/*
 * ECC_comm.c
 *
 *  Created on: 2014/10/10
 *      Author: RyzeVia
 */

#include "egc_sys.h"


static EGC_COMM_MODE SPEGC_comm_convertmode(char* buf);

void PEGC_comm_init(){
	EGC_PRE_DBG("INTERNAL");
	PEGC_comm_sock_init(&(commtable[EGC_COMM_MODE_SOCKET]));
	PEGC_comm_file_init(&(commtable[EGC_COMM_MODE_FILE]));

	INIT_LIST(&intercomm);
	INIT_LIST(&intracomm);

	DEBUGF(DEBUG_LEVEL_INFO, "INFO: intercomm was initialized address:%p -> %p -> %p\n", intercomm.prev, &intercomm, intercomm.next);
	DEBUGF(DEBUG_LEVEL_INFO, "INFO: intracomm was initialized address:%p -> %p -> %p\n", intracomm.prev, &intracomm, intracomm.next);


	EGC_POST_DBG("INTERNAL");
}

/*
void PEGC_comm_bufalloc(p_msgbuf *mbuf, int reserved_bufsize){
	EGC_PRE_DBG("INTERNAL");

	INIT_LIST(&(mbuf->wqueue));
	MALLOC(char*, mbuf->buf, sizeof(p_msginfo)+reserved_bufsize);


	EGC_POST_DBG("INTERNAL");
}
*/
void PEGC_comm_bufrealloc(p_msgbuf *mbuf, int reserved_bufsize){
	EGC_PRE_DBG("INTERNAL");

	INIT_LIST(&(mbuf->wqueue));
	REALLOC(char*, mbuf->buf, sizeof(p_msginfo)+mbuf->size, sizeof(p_msginfo)+reserved_bufsize);
	mbuf->size = reserved_bufsize;

	EGC_POST_DBG("INTERNAL");
}


p_commhdr* PEGC_comm_newhdr_blank(EGC_COMM_MODE mode){
	p_commhdr *hdr;
	p_totimer to = {-1, {DEF_EGC_TOSEC, 0}};

	EGC_PRE_DBG("INTERNAL");
	MALLOC(p_commhdr*, hdr, sizeof(p_commhdr));

	hdr->mode = mode;
//	hdr->desthdr = NULL;
	hdr->fd = -1;
	hdr->to = to;
	hdr->call = &(commtable[mode]);
	hdr->minfo.destid = -1;
	hdr->minfo.srcid = -1;
	hdr->minfo.nbytes = -1;
	hdr->minfo.messageid = -1;
	hdr->state = EGC_COMM_STATE_NULL;

	INIT_LIST(&(hdr->hdrchain));

	hdr->ev = NULL;
	hdr->toev = NULL;
	hdr->mbuf = NULL;

	EGC_POST_DBG("INTERNAL");
	return hdr;
}

p_commhdr* PEGC_comm_newhdr(EGC_COMM_MODE mode, int reserved_bufsize, int tosec){
	p_commhdr *hdr;
	p_totimer to = {-1, {tosec, 0}};

	EGC_PRE_DBG("INTERNAL");

	hdr = PEGC_comm_newhdr_blank(mode);
	hdr->mbuf = hdr->call->bufalloc(reserved_bufsize);
	hdr->to = to;

	EGC_POST_DBG("INTERNAL");
	return hdr;
}

void PEGC_comm_freehdr(p_commhdr* hdr){
	EGC_PRE_DBG("INTERNAL");
	if(hdr != NULL){
		if(hdr->mbuf->buf != NULL){
			free(hdr->mbuf->buf);
		}
		free(hdr);
	}
	EGC_POST_DBG("INTERNAL");
}

void PEGC_comm_addhdrchain(p_commhdr* hdr, LIST_HOLDER* holder){
	LIST_ADD_TAIL(&(hdr->hdrchain), holder);
	DEBUGF(DEBUG_LEVEL_INFO, "INFO: FD %d, keyholder renewal address:%p -> %p -> %p\n", hdr->fd, holder->prev, holder, holder->next);
}

p_commhdr* PEGC_comm_getnextchain(p_commhdr* hdr){
	p_commhdr* rvhdr;
	rvhdr = LIST_ENTRY(hdr->hdrchain.next, p_commhdr, hdrchain);
	return rvhdr;
}


void PEGC_comm_set_bursterror_timer(p_commhdr* hdr, struct timeval *tv){
	if(tv != NULL){
		hdr->to.tv = *tv;
	}
	hdr->to.use = 1;
}

void PEGC_comm_clear_bursterror_timer(p_commhdr* hdr){
	hdr->to.use = -1;
}

p_commhdr* PEGC_intercomm_gethdr(p_commhdr *from_hdr){
	//EXAM: �z�X�g����ID���ӂ�A�����z�X�g�Ƃ̒ʐM���l������ɂ͂����ɒǉ��A�܂��}���`�]����������
	p_commhdr *rv;
	rv = LIST_ENTRY(intercomm.next, p_commhdr, hdrchain);
	DEBUGF(DEBUG_LEVEL_INFO, "INFO: intercomm_gethdr fd %d\n", rv->fd);
	return rv;
}

p_commhdr* PEGC_intracomm_gethdr(p_commhdr *readhdr){
	p_commhdr *rv = NULL;
	p_msginfo *packet;
//	p_msginfo lpacket;

	packet = (p_msginfo*) (readhdr->mbuf->buf);
//	lpacket.srcid = ntohl(packet->srcid);
//	lpacket.destid = ntohl(packet->destid);

	rv = PEGC_comm_gethdr_w_uid(packet->srcid, packet->destid, &intracomm);

	if(rv == NULL){
		rv = PEGC_comm_newhdr_blank(PEGC_intracomm_getmode(PEGCU_get_jobid()));
		rv->minfo = *packet;
		rv->mbuf = rv->call->bufalloc(0);
		rv->call->adhoc_open_minfo(rv);
		PEGC_comm_addhdrchain(rv, &intracomm);
	}

	return rv;
}

p_commhdr* PEGC_comm_gethdr_w_uid(int src, int dest, LIST_HOLDER* commlist){
	LIST_HEAD *lc;
	p_commhdr *rv = NULL;

	EGC_PRE_DBG("INTERNAL");

	DEBUGF(DEBUG_LEVEL_INFO, "INFO: seek DP with src:%d, des:%d form %p\n", src, dest, commlist);
	if(commlist == commlist->prev) {
		goto ret_comm_gethdr_w_uid;
	}

	for(lc = commlist->next; lc != commlist; lc = lc->next){
		rv = LIST_ENTRY(lc, p_commhdr, hdrchain);
		DEBUGF(DEBUG_LEVEL_INFO, "INFO: check DP %p, src:%d, dest:%d\n", rv, rv->minfo.srcid, rv->minfo.destid);
		if((rv->minfo.destid == dest) && (rv->minfo.srcid == src))
			break;
	}

	if(lc == commlist) rv = NULL;

ret_comm_gethdr_w_uid:

	EGC_POST_DBG("INTERNAL");
	return rv;

}


// intercomm ��TCP/IP������
void PEGC_intercomm_cblistener(evutil_socket_t fd, short ef, void* args){
	p_commhdr *hdr;
	p_commhdr *dhdr;
	int bufsize;

	EGC_PRE_DBG("INTERNAL");

	hdr = PEGC_event_getcommhdr(args);
	dhdr = PEGC_comm_newhdr_blank(EGC_COMM_MODE_SOCKET);
	hdr->call->accept(dhdr, hdr);
	bufsize = CFR_getvalue_byint("EGC_PACKET_SIZE", DEF_EGC_PACKET_SIZE);

	//���M���A�t�@�C���ǂݍ��݂ƌ������邽�߂̃o�b�t�@
	dhdr->mbuf = dhdr->call->bufalloc(bufsize);

	dhdr->to.use = -1; //�A���Ăяo���~�X�ɂ��^�C���A�E�g�͐ݒ肵�Ȃ� (Socket �ł͋N���Ȃ�)
	PEGC_comm_addhdrchain(dhdr, &intercomm); //���M�n���h���̓o�^

	/* EXAM: �z�X�g����ID���ӂ�A�����z�X�g�Ƃ̒ʐM���l������ɂ͂����ɒǉ�
	 * ���[�U����z�X�gID���w�肵�Ă��炤�K�v������
	 */

	//char greeting[MAX_EGC_GREETING_PKT];
	//sprintf(greeting, "%s:%s", localhost.uid, localhost.pass);
	//BWRITE(dhdr->fd, greeting, MAX_EGC_GREETING_PKT);
	//DEBUGF(DEBUG_LEVEL_INFO, "INFO: send greeting msg to %d: %s\n", dhdr->fd, greeting);
	//write(dhdr->fd, greeting, MAX_EGC_GREETING_PKT);
	//	PEGC_event_regist(NULL, dhdr, PEGC_intercomm_cbdata); // ACK ������K�v

	PEGC_event_register_again(hdr);

	EGC_POST_DBG("INTERNAL");

}

void PEGC_intercomm_cbdata(evutil_socket_t fd, short ef, void* args){
	p_commhdr *hdr, *ldhdr;
	EGC_COMM_STATE rv;
	char* cbuf;

	EGC_PRE_DBG("INTERNAL");

	//FIXME(LATER): �f�[�^�󂯎�苒�_���n���h��

	hdr = PEGC_event_getcommhdr(args);

	rv = hdr->call->inter_read(hdr);

	//test code
	//int i;
	//for(i = 0; i < 50; i++){
	//	fprintf(stderr, "%d\n", ((int*)hdr->mbuf->buf)[i]);
	//}


	ldhdr = PEGC_intracomm_gethdr(hdr);

	DEBUGF(DEBUG_LEVEL_INFO, "INFO: hdr %d got data and send to %d\n", hdr->fd, ldhdr->fd);

	cbuf = hdr->mbuf->buf;
	hdr->mbuf->buf += sizeof(p_msginfo);
	ldhdr->call->intra_write(ldhdr, hdr);
	hdr->mbuf->buf = cbuf;

	PEGC_event_register_again(hdr);

	EGC_POST_DBG("INTERNAL");
}

void PEGC_intracomm_cblistener(evutil_socket_t fd, short ef, void* args){
	p_commhdr *hdr;
	p_commhdr *dhdr;
	EGC_COMM_STATE st;

	EGC_PRE_DBG("INTERNAL");
	hdr = PEGC_event_getcommhdr(args);

	//FIXME: Protocol �����ߑł��A�I���ł���悤�ɒ���
	dhdr = PEGC_comm_newhdr_blank(EGC_COMM_MODE_FILE);
/*
	int bufsize;
	bufsize = CFR_getvalue_byint("EGC_PACKET_SIZE", DEF_EGC_PACKET_SIZE);
	dhdr->mbuf = dhdr->call->bufalloc(bufsize);
*/
	st = hdr->call->accept(dhdr, hdr);

	//	PEGC_comm_addhdrchain(dhdr, intracomm);

	switch(st){
	case EGC_COMM_STATE_ESTABLISH:
		PEGC_event_register(hdr->ehdr, dhdr, EV_READ, PEGC_intracomm_cbdata);
		PEGC_event_register_again(hdr);
		break;
	case EGC_COMM_STATE_UNCHANGE:
		// �ʐM�`���� File �̏ꍇ�́A�S�Ẵt�@�C����1�C�x���g�ׁ̈A�V���ȃC�x���g����������Ȃ�
		// Socket �̏ꍇ�͋N���肦�Ȃ�����{�I�ɂ͉������邱�Ƃ��Ȃ�
		PEGC_event_register_again(hdr);
		break;
	case EGC_COMM_STATE_TIMEOUT:
		// timeout timer �C�x���g�̗L����
		PEGC_event_register_timer(hdr);
		break;
	case EGC_COMM_STATE_NULL:
		ERRORF("(TYPE %d):accept error\n", hdr->mode);
		ERROR_EXIT();
		break;
	default:
		break;
	}

	EGC_POST_DBG("INTERNAL");
}

void PEGC_intracomm_cbdata(evutil_socket_t fd, short ef, void* args){
	p_commhdr *hdr;
	p_commhdr *dsthdr;
	p_commhdr *mhdr;
	EGC_COMM_HDL rv;
	EGC_PRE_DBG("INTERNAL");

	mhdr = hdr = PEGC_event_getcommhdr(args);
	if(hdr->mbuf == NULL){
		hdr = LIST_ENTRY(intracomm.next, p_commhdr, hdrchain);
	}
	dsthdr = PEGC_intercomm_gethdr(hdr);
	do{
		rv = hdr->call->intra_read(hdr);
		switch(rv){
		case EGC_COMM_HDL_OK: //����ǂݍ��ݏI��
			//����ꂽ�f�[�^��intercomm �ɑ����Ă��܂��i�u���b�L���O�ł������j
			DEBUGF(DEBUG_LEVEL_INFO, "INFO: intracomm_cbdata write to: %d\n", dsthdr->fd);

			/* test code */
			//fwrite(hdr->mbuf->buf, 9600, 1, stderr);


			dsthdr->call->inter_write(dsthdr, hdr);
			// cleanup �� write ���Ɏ��߂Ă���i�u���b�L���Owrite �Ή�)
			// hdr->call->rcleanup(hdr);
			break;
		case EGC_COMM_HDL_AGAIN:
			hdr->call->ragain(hdr);
			break;
		case EGC_COMM_HDL_WAIT:
			PEGC_event_register_timer(hdr);
			break;
		case EGC_COMM_HDL_INPROGRESS:
			break;
		default:
			break;
		}
		hdr = hdr->call->intra_nexthdr(hdr);
	}while(&(hdr->hdrchain) != &intracomm);

	PEGC_event_register_again(mhdr);

	EGC_POST_DBG("INTERNAL");
}



void PEGC_comm_cbwaittimer(evutil_socket_t fd, short ef, void* args){
	p_commhdr *hdr;
	EGC_COMM_STATE state;

	EGC_PRE_DBG("INTERNAL");
/* timer �� ���C���C�x���g�� args �͋��ʂ��Ă��� */
	hdr = PEGC_event_getcommhdr(args);
	state = hdr->call->rwait(hdr);
//	if (state == EGC_COMM_STATE_ESTABLISH){
		PEGC_event_register_again(hdr);
//	}
	EGC_POST_DBG("INTERNAL");
}

EGC_COMM_MODE PEGC_intracomm_getmode(int jid){
	char wsbuf[MAX_EGC_KEYVALUE_SIZE];
	char* lmode;
	sprintf(wsbuf, "EGC_INTRA_MODE%d", jid);
	lmode = CFR_getvalue(wsbuf, DEF_EGC_INTRALMODE);

	return SPEGC_comm_convertmode(lmode);
}

EGC_COMM_MODE PEGC_intercomm_getmode(int jid){
	char wsbuf[MAX_EGC_KEYVALUE_SIZE];
	char* lmode;
	sprintf(wsbuf, "EGC_INTER_MODE%d", jid);
	lmode = CFR_getvalue(wsbuf, DEF_EGC_INTERLMODE);

	return SPEGC_comm_convertmode(lmode);
}

static EGC_COMM_MODE SPEGC_comm_convertmode(char* buf){
	int rv = -1;

	if(strncmp("file", buf, 4) == 0){
		rv = EGC_COMM_MODE_FILE;
	}
	else if(strncmp("socket", buf, 6) == 0){
		rv = EGC_COMM_MODE_SOCKET;
	}
	else{
		ERRORF("Unexpected COMM Mode: %s\n", buf);
		ERROR_EXIT();
	}

	return rv;
}





// ???
