/*
 * EGC_comm_sock.c
 *
 *  Created on: 2014/11/13
 *      Author: RyzeVia
 */

#include "egc_sys.h"

static void SPEGC_comm_sock_setcall(p_commcall* call);

int PEGC_comm_sock_init(p_commcall* call) {
	EGC_PRE_DBG("INTERNAL");
	SPEGC_comm_sock_setcall(call);
	EGC_POST_DBG("INTERNAL");
	return 0;
}

static void SPEGC_comm_sock_setcall(p_commcall* call) {
	call->type = EGC_COMM_MODE_SOCKET;
	call->listen = PEGC_comm_sock_listen;
	call->accept = PEGC_comm_sock_accept;
	call->connect = PEGC_comm_sock_connect;
	call->bufalloc = PEGC_comm_sock_bufalloc;
	call->inter_read = PEGC_intercomm_sock_read;
	call->inter_write = PEGC_intercomm_sock_write;
	call->intra_read = PEGC_intracomm_sock_read;
	call->intra_write = PEGC_intracomm_sock_write;
	call->intra_nexthdr = NULL;
	call->rcleanup = NULL;
	call->ragain = NULL;
	call->rwait = NULL;

}

p_msgbuf* PEGC_comm_sock_bufalloc(int bufsize) {
	p_msgbuf *buf;
	EGC_PRE_DBG("INTERNAL");

	MALLOC(p_msgbuf*, buf, sizeof(p_msgbuf));
	INIT_LIST(&(buf->wqueue));
	//	buf->size = sizeof(p_msginfo) + bufsize;
	buf->size = bufsize;
	MALLOC(char*, buf->buf, buf->size);
	MALLOC(struct siob*, buf->ext.sio, sizeof(struct siob));

	buf->status = EGC_MSG_STATE_NOWORK;
	buf->ext.sio->offset = 0;

	EGC_POST_DBG("INTERNAL");
	return buf;
}

void PEGC_comm_sock_buffree(p_msgbuf* buf) {
	EGC_PRE_DBG("INTERNAL");
	free(buf->buf);
	free(buf);
	EGC_POST_DBG("INTERNAL");
}

EGC_COMM_STATE PEGC_comm_sock_listen(p_commhdr *hdr) {
	struct sockaddr_in saddr;
	int rv;
	EGC_PRE_DBG("INTERNAL");

	hdr->addr.name[0] = '\0'; //assertion ?
	hdr->fd = socket(AF_INET, SOCK_STREAM, 0);
	ERRCHK(hdr->fd < 0, "socket");

	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = INADDR_ANY;
	saddr.sin_port = htons(hdr->addr.portnum);

	DEBUGF(DEBUG_LEVEL_INFO, "INFO: listen to %d (socknum=%d)\n", hdr->addr.portnum, hdr->fd);

	rv = bind(hdr->fd, (struct sockaddr*) &saddr, sizeof(struct sockaddr_in));
	ERRCHK(rv < 0, "bind");

	rv = listen(hdr->fd, SOMAXCONN);
	ERRCHK(rv < 0, "listen");

	EGC_POST_DBG("INTERNAL");

	return EGC_COMM_STATE_ESTABLISH;
}

EGC_COMM_STATE PEGC_comm_sock_accept(p_commhdr *nhdr, p_commhdr *hdr) {
	struct sockaddr_in caddr;
	int len = sizeof(struct sockaddr_in);

	EGC_PRE_DBG("INTERNAL");


	nhdr->fd = accept(hdr->fd, (struct sockaddr *) &caddr, (socklen_t *) &len);
	ERRCHK(nhdr->fd < 0, "accept");

	DEBUGF(DEBUG_LEVEL_INFO, "INFO: Socket Accept %d from %d\n", nhdr->fd, hdr->fd);

	memcpy(nhdr->addr.name, &caddr.sin_addr, MAX_EGC_HOSTLEN);
	nhdr->addr.portnum = caddr.sin_port;

	EGC_POST_DBG("INTERNAL");
	return EGC_COMM_STATE_ESTABLISH;
}

EGC_COMM_STATE PEGC_comm_sock_connect(p_commhdr *hdr) {
	int rv;
	struct sockaddr_in caddr;
	int len = sizeof(struct sockaddr_in);
	int optval = 1;
	struct hostent *hp;

	EGC_PRE_DBG("INTERNAL");

	hp = gethostbyname(hdr->addr.name);
	ERRCHK(hp == NULL, "gethostbyname");
	memcpy((void*)&caddr.sin_addr, (void*)hp->h_addr, hp->h_length);
	caddr.sin_family = hp->h_addrtype;
	caddr.sin_port = htons(hdr->addr.portnum);


	DEBUGF(DEBUG_LEVEL_INFO, "INFO: connect to %s port %d\n", hdr->addr.name,
			hdr->addr.portnum);

	hdr->fd = socket(AF_INET, SOCK_STREAM, 0);
	ERRCHK(hdr->fd < 0, "socket");

	rv = setsockopt(hdr->fd, IPPROTO_TCP, TCP_NODELAY, (char *) &optval,
			sizeof(optval ));


	rv = connect(hdr->fd, (struct sockaddr*) &caddr, len);
	ERRCHK(rv < 0, "connect");

	DEBUGF(DEBUG_LEVEL_INFO, "INFO: connected by %d\n", hdr->fd);

	EGC_POST_DBG("INTERNAL");
	return EGC_COMM_STATE_ESTABLISH;
}

EGC_COMM_HDL PEGC_intercomm_sock_read(p_commhdr* hdr) {
	int rv;
	char* buf = hdr->mbuf->buf;
	int size = hdr->mbuf->size;
	EGC_PRE_DBG("INTERNAL");

	DEBUGF(DEBUG_LEVEL_INFO, "INFO: inter read expected size %d\n", size);
	rv = BREAD(hdr->fd, buf, size);

	p_msginfo* minfo = (p_msginfo*)buf;
	DEBUGF(DEBUG_LEVEL_INFO, "INFO: recv from remote, src=%d, dest=%d, id = %d, size=%d\n", minfo->srcid, minfo->destid, minfo->messageid, minfo->nbytes);
//	hdr->minfo = *minfo;
	PEGCU_ntohl_minfo(&hdr->minfo, minfo);
	PEGCU_ntohl_minfo(minfo, minfo);
	DEBUGF(DEBUG_LEVEL_INFO, "INFO: converted, src=%d, dest=%d, id = %d, size=%d\n", minfo->srcid, minfo->destid, minfo->messageid, minfo->nbytes);
	DEBUGF(DEBUG_LEVEL_INFO, "INFO: converted confirm, src=%d, dest=%d, id = %d, size=%d\n", hdr->minfo.srcid, hdr->minfo.destid, hdr->minfo.messageid, hdr->minfo.nbytes);


	EGC_POST_DBG("INTERNAL");
	return EGC_COMM_HDL_OK;
}
EGC_COMM_HDL PEGC_intercomm_sock_write(p_commhdr *dsthdr, p_commhdr *hdr) {
	int rv;
//	p_msgbuf *mbuf = hdr->mbuf;
	char* buf = hdr->mbuf->buf;
	int size = hdr->mbuf->size;
	EGC_PRE_DBG("INTERNAL");

	DEBUGF(DEBUG_LEVEL_INFO, "INFO: inter_write(sock): %d\n", dsthdr->fd);

	//ブランクバッファと交換ののち、初期化＋再読み込み開始
	//write が終わるまではイベントは発行されない（再登録してないので）
	hdr->mbuf->buf = dsthdr->mbuf->buf;
	dsthdr->mbuf->buf = buf;
	hdr->call->rcleanup(hdr);

	/* test code */
	//fwrite(buf, 9600, 1, stderr);
	p_msginfo *minfo = (p_msginfo*)buf;
	DEBUGF(DEBUG_LEVEL_INFO, "INFO: send to remote, src=%d, dest=%d, id = %d, size=%d\n", minfo->srcid, minfo->destid, minfo->messageid, minfo->nbytes);
	PEGCU_htonl_minfo(minfo, minfo);
	DEBUGF(DEBUG_LEVEL_INFO, "INFO: converted, src=%d, dest=%d, id = %d, size=%d\n", minfo->srcid, minfo->destid, minfo->messageid, minfo->nbytes);

	// 交換した読み込みバッファをブロッキングwrite
	// ブロッキングでもシグナルが来ると中断はあり得る
	rv = BWRITE(dsthdr->fd, buf, size);


	EGC_POST_DBG("INTERNAL");
	return EGC_COMM_HDL_OK;
}

EGC_COMM_HDL PEGC_intracomm_sock_read(p_commhdr* hdr) {
	EGC_PRE_DBG("INTERNAL");
	assert(0); //未実装
	EGC_POST_DBG("INTERNAL");
	return EGC_COMM_HDL_OK;
}

EGC_COMM_HDL PEGC_intracomm_sock_write(p_commhdr* dsthdr, p_commhdr* hdr) {
	EGC_PRE_DBG("INTERNAL");
	assert(0); //未実装
	EGC_POST_DBG("INTERNAL");
	return EGC_COMM_HDL_OK;

}

