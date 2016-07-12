/*
 * ECC_comm_file.c
 *
 *  Created on: 2014/11/13
 *      Author: RyzeVia
 */

#include "egc_sys.h"
#define LISTENER_BUFSIZE	(32);

typedef enum {
	EGC_FILE_AIO_ISSUED,
	EGC_FILE_AIO_NONISSUED,
	MAX_EGC_FILE_AIO,
} p_aiostate;

static void SPEGC_comm_file_setcall(p_commcall* call);
static int SPEGC_comm_file_getsignalfd(int signalno);
static EGC_COMM_HDL SPEGC_comm_file_aio_eofcheck(p_commhdr *hdr);
static int SPEGC_comm_file_open(p_commhdr *hdr);
static void SPEGC_comm_file_signalreflesh(p_commhdr *hdr);


int PEGC_comm_file_init(p_commcall* call){

	EGC_PRE_DBG("INTERNAL");

	SPEGC_comm_file_setcall(call);

	EGC_POST_DBG("INTERNAL");

	return EXIT_SUCCESS;
}

static void SPEGC_comm_file_setcall(p_commcall* call){
	call->type = EGC_COMM_MODE_FILE;
	call->listen = PEGC_comm_file_listen;
	call->accept = PEGC_comm_file_accept;
	call->connect = PEGC_comm_file_connect;
	call->bufalloc = PEGC_comm_file_bufalloc;
	call->inter_read = NULL;
	call->inter_write = NULL;
	call->intra_read = PEGC_comm_file_read;
	call->intra_write = PEGC_intracomm_file_write;
	call->intra_nexthdr = PEGC_intracomm_file_nexthdr;
	call->rcleanup = PEGC_comm_file_rcleanup;
	call->ragain = PEGC_comm_file_ragain;
	call->rwait = PEGC_comm_file_rwait;

	call->adhoc_open_minfo = PEGC_intracomm_adhoc_open_minfo;

}

static int SPEGC_comm_file_getsignalfd(int signalno){
	sigset_t mask;

	EGC_PRE_DBG("INTERNAL");

	sigemptyset(&mask);
	sigaddset(&mask, signalno);
	ERRCHK(sigprocmask(SIG_BLOCK, &mask, NULL) == -1, "comm_file_init:sigprocmask");

	sigemptyset(&mask);
	sigaddset(&mask, signalno);

	return signalfd(-1, &mask, 0);
}

p_commhdr* PEGC_intracomm_file_nexthdr(p_commhdr* hdr){
	p_commhdr* rv;
	rv = LIST_ENTRY(hdr->hdrchain.next, p_commhdr, hdrchain);
	return rv;
}


p_msgbuf* PEGC_comm_file_bufalloc(int bufsize){
	p_msgbuf *buf;
	struct aiocb* aio;
	EGC_PRE_DBG("INTERNAL");

	MALLOC(p_msgbuf*, buf, sizeof(p_msgbuf));
	INIT_LIST(&(buf->wqueue));
	//	buf->size = sizeof(p_msginfo)+bufsize;
	buf->size = bufsize;
	MALLOC(char*, buf->buf, buf->size);
	buf->status = EGC_MSG_STATE_NOWORK;
	MALLOC(struct aiocb*, buf->ext.aio, sizeof(struct aiocb));

	aio = buf->ext.aio;
	aio->aio_fildes = -1;
	aio->aio_buf = buf->buf;
	aio->aio_offset = 0;
	aio->aio_nbytes = buf->size;
	aio->aio_reqprio = 0;
	aio->aio_sigevent.sigev_notify = SIGEV_SIGNAL;
	aio->aio_sigevent.sigev_signo = -1;

	EGC_POST_DBG("INTERNAL");
	return buf;
}

void PEGC_comm_file_buffree(p_msgbuf* buf){
	EGC_PRE_DBG("INTERNAL");
	free(buf->buf);
	free(buf);
	EGC_POST_DBG("INTERNAL");
}


EGC_COMM_STATE PEGC_comm_file_listen(p_commhdr *hdr){
	static int fd = -1;

	EGC_PRE_DBG("INTERNAL");

	if(fd == -1){
		aio_listen_signal = CFR_getvalue_byint("EGC_INTRA_SIG_LISTEN", DEF_AIO_SIG_LISTEN);
		fd = SPEGC_comm_file_getsignalfd(aio_listen_signal);
	}
	hdr->fd = fd;
/*
	hdr->mbuf.status = EGC_MSG_STATE_NOWORK;
	hdr->mbuf.offset = 0;
	hdr->mbuf.size = EGC_FLISTEN_BUF;
	MALLOC(char*, hdr->mbuf.buf, EGC_FLISTEN_BUF);
	MALLOC(struct aiocb*, hdr->ext.aio, sizeof(struct aiocb));
*/
	hdr->mbuf = PEGC_comm_file_bufalloc(EGC_FLISTEN_BUF);
	hdr->mbuf->ext.aio->aio_sigevent.sigev_signo = aio_listen_signal;
	SPEGC_comm_file_open(hdr);
	aio_read(hdr->mbuf->ext.aio);
	hdr->mbuf->status = EGC_FILE_AIO_ISSUED;
	EGC_POST_DBG("INTERNAL");
	return EGC_COMM_STATE_ESTABLISH;
}

// クライアントから使われる - aio じゃない
/*
EGC_COMM_STATE PEGC_comm_file_connect(p_commhdr *hdr){
	static int fd = -1;

	EGC_PRE_DBG("INTERNAL");

	hdr->fd = fd;
	hdr->mbuf->status = EGC_MSG_STATE_NOWORK;
	MALLOC(char*, hdr->mbuf->buf, EGC_FLISTEN_BUF);
	MALLOC(struct aiocb*, hdr->mbuf->ext.aio, sizeof(struct aiocb));
	SPEGC_comm_file_open(hdr); //リスナのオープン


	//EXAM: connect 当座実装、後で再チェック

	EGC_POST_DBG("INTERNAL");
	return fd;
}
*/

/* PEGC_comm_file_connect
 *
 */
EGC_COMM_STATE PEGC_comm_file_connect(p_commhdr *hdr){
	int fd;
	EGC_PRE_DBG("INTERNAL");

	fd = SPEGC_comm_file_open(hdr);

//	rv = write(fd, hdr->mbuf->buf, EGC_FLISTEN_BUF);

	// send datapath name to listener file
	// Listener file was shared, then it need lock.


	BWRITE(fd, hdr->addr.name, EGC_FLISTEN_BUF);

// Open Data file
	hdr->addr.portfile = NULL;

	hdr->fd = SPEGC_comm_file_open(hdr);

/*
	DEBUGF(DEBUG_LEVEL_INFO, "INFO: open file with name:%s\n", fn);
	hdr->fd = open(hdr->addr.name, O_RDWR | O_CREAT, S_IRWXU);
	ERRCHK(hdr->fd < 0, "comm_file_connect:");
*/
	EGC_POST_DBG("INTERNAL");

	return EGC_COMM_STATE_ESTABLISH;

}


/* PEGC_comm_file_accept: file を使った通信 accept
 * o/EGC_COMM_STATE: 作成された新しいハンドラの状態
 * 		EGC_COMM_STATE_ESTABLISH: accept し新しいハンドラを作った
 * 		EGC_COMM_STATE_UNCHANGE: accept の有無にかかわらずハンドラは作られなかった
 * 		EGC_COMM_STATE_TIMEOUT: accept すべきクエりが来ていない
 * o/p_commhdr *nhdr: accept の結果作られた新しいハンドラ
 * i/p_commhdr *hdr: listen しているハンドラ
 */
EGC_COMM_STATE PEGC_comm_file_accept(p_commhdr *nhdr, p_commhdr *hdr){
	EGC_COMM_HDL rv;
	p_commhdr *shdr;
	static int accepted = 0;
	int bufsize;

	EGC_PRE_DBG("INTERNAL");

	rv = PEGC_comm_file_read(hdr);
	switch(rv){
	case EGC_COMM_HDL_WAIT:
		DEBUGF(DEBUG_LEVEL_INFO, "INFO: aio waiting on %d with buf:%s\n", hdr->mbuf->ext.aio->aio_fildes, hdr->mbuf->buf);
		rv = EGC_COMM_STATE_TIMEOUT;
		break;
	case EGC_COMM_HDL_OK:
		if(accepted == 0){
			aio_data_signal = CFR_getvalue_byint("EGC_INTRA_SIG_DATA", DEF_AIO_SIG_DATA);
			nhdr->fd = SPEGC_comm_file_getsignalfd(aio_data_signal);
			DEBUGF(DEBUG_LEVEL_INFO, "INFO: watching start new signal fd: %d\n", nhdr->fd);
			rv = EGC_COMM_STATE_ESTABLISH;
		}else{
			rv = EGC_COMM_STATE_UNCHANGE;
		}

		shdr = PEGC_comm_newhdr_blank(EGC_COMM_MODE_FILE);
		bufsize = CFR_getvalue_byint("EGC_PACKET_SIZE", DEF_EGC_PACKET_SIZE);
		shdr->mbuf = shdr->call->bufalloc(bufsize);
		sprintf(shdr->addr.name, "%s", hdr->mbuf->buf);
		shdr->addr.portfile = NULL;

		//shdr->fd =
		SPEGC_comm_file_open(shdr);
		// fd はシグナルfd, AIOリードは shdr->mbuf->ext.aio 内にある
		shdr->fd = nhdr->fd;

		shdr->mbuf->ext.aio->aio_sigevent.sigev_signo = aio_data_signal;
		aio_read(shdr->mbuf->ext.aio);
		shdr->mbuf->status = EGC_FILE_AIO_ISSUED;
		hdr->mbuf->buf[0] = '\0';
		PEGC_comm_addhdrchain(shdr, &intracomm);

		accepted++;
		DEBUGF(DEBUG_LEVEL_INFO, "INFO: accepted new file:%d\n", accepted);
		PEGC_comm_file_rcleanup(hdr);
		break;
	case EGC_COMM_HDL_AGAIN:
		PEGC_comm_file_ragain(hdr);
		rv = EGC_COMM_STATE_UNCHANGE;
		break;
	case EGC_COMM_HDL_INPROGRESS:
		rv = EGC_COMM_STATE_UNCHANGE;
		break;
	default:
		ERRORF("unexpected error\n");
		ERROR_EXIT();
	}
	EGC_POST_DBG("INTERNAL");

	return rv;
}

/* PEGC_comm_file_read: file を使った通信 read
 * o/EGC_COMM_HDL: 読み込みハンドラの状態
 * 		EGC_COMM_HDL_OK: 予定ブロックを読み込んだ
 * 		EGC_COMM_HDL_AGAIN: 読み込み終了だが予定ブロックはまだ届いていない（文末）、aio_read 再発行画必要
 * 		EGC_COMM_HDL_WAIT: 二度目の文末、少しハンドラの読み込みを待つ (aio_read の再発行を遅らせる)
 * 		EGC_COMM_HDL_INPROGRESS: 読み込み中
 */
EGC_COMM_HDL PEGC_comm_file_read(p_commhdr *hdr){
	int status;
	EGC_COMM_STATE rv;

	EGC_PRE_DBG("INTERNAL");
	if(hdr->mbuf->status == EGC_FILE_AIO_NONISSUED)
		goto comm_file_read_fin;

	status = aio_error(hdr->mbuf->ext.aio);
	switch(status){
	case EINPROGRESS:
		//おそらくここが２回来ることはないと思うんだけど
//		rv = SPEGC_comm_file_renew_hdlstate(hdr, EGC_COMM_HDL_AGAIN);
//		DEBUGF(DEBUG_LEVEL_INFO, "INFO: AIO_ERROR return EINPROGRESS on %d\n", hdr->mbuf->ext.aio->aio_fildes);
		rv = EGC_COMM_HDL_INPROGRESS;
		break;
	case ECANCELED:
		ERRORF("AIO canceled: Unexpected cancel\n");
		ERROR_EXIT();
		break;
	case 0:
		DEBUGF(DEBUG_LEVEL_INFO, "INFO: AIO_ERROR return 0(complete) on %d\n", hdr->mbuf->ext.aio->aio_fildes);
		rv = SPEGC_comm_file_aio_eofcheck(hdr);
		DEBUGF(DEBUG_LEVEL_INFO, "INFO: AIO_EOFCHECK return %d on %d\n", rv, hdr->mbuf->ext.aio->aio_fildes);
		hdr->mbuf->status = EGC_FILE_AIO_NONISSUED;
		SPEGC_comm_file_signalreflesh(hdr);
		break;
	default:
		errno = status;
		PERROR_LOC("aio:");
		ERROR_EXIT();
		break;
	}

comm_file_read_fin:

	EGC_POST_DBG("INTERNAL");
	return rv;
}

static void SPEGC_comm_file_signalreflesh(p_commhdr *hdr){
	struct signalfd_siginfo sinfo;
	read(hdr->fd, (char*)&sinfo, sizeof(struct signalfd_siginfo));
	DEBUGF(DEBUG_LEVEL_INFO, "INFO: SIGNAL %d refleshed for %d\n", sinfo.ssi_signo, hdr->fd);
}


EGC_COMM_HDL PEGC_intracomm_file_write(p_commhdr* dsthdr, p_commhdr* hdr){
	int fd = dsthdr->fd;
	char* buf = hdr->mbuf->buf;
//	p_msginfo htonlinfo;
//	char* minfo = (char*)&htonlinfo;
	char* minfo = (char*)&(hdr->minfo);



	int psize = CFR_getvalue_byint("EGC_PACKET_SIZE", DEF_EGC_PACKET_SIZE);
	int hsize = sizeof(p_msginfo);
	int csize = psize - hsize;

	/* fake buffer */
	static char* fake = NULL;

	EGC_PRE_DBG("INTERNAL");

	if(fake == NULL){
		MALLOC(char*, fake, csize);
	}

//	p_msginfo *mi = (p_msginfo*)minfo;
//	DEBUGF(DEBUG_LEVEL_INFO, "INFO: minfo=(src:%d, dest:%d, size:%d, id:%d)\n", hdr->minfo.srcid, hdr->minfo.destid, hdr->minfo.nbytes, hdr->minfo.messageid);
//	DEBUGF(DEBUG_LEVEL_INFO, "INFO: write to file, minfo=(src:%d, dest:%d, size:%d, id:%d)\n", mi->srcid, mi->destid, mi->nbytes, mi->messageid);

	BWRITE(fd, minfo, hsize);

	if(csize < hdr->minfo.nbytes){
		BWRITE(fd, buf, csize);
	}else{
		BWRITE(fd, buf, hdr->minfo.nbytes);
		// Padding
		BWRITE(fd, fake, csize-hdr->minfo.nbytes);
	}



	EGC_POST_DBG("INTERNAL");

	return EGC_COMM_HDL_OK;
}



/* SPEGC_comm_file_aio_eofcheck:
 * 非同期読み込み結果取得およびファイルの読み込み中／終了／データ未到達かのチェック
 * aio_error が呼び出された後、只一度のみ呼び出せる。
 * o/EGC_COMM_HDL: ハンドラの読み込み状態
 *		EGC_COMM_HDL_OK: ファイル読み込み終了
 *		EGC_COMM_HDL_AGAIN: 読むべきデータが残っている(再実行求む)
 *		EGC_COMM_HDL_WAIT: データ未到達(文末で読み込み0が繰り返される)
 *　i/p_commhdr *hdr: 読み込み状態をチェックするハンドラ
 */
static EGC_COMM_HDL SPEGC_comm_file_aio_eofcheck(p_commhdr *hdr){
	EGC_COMM_HDL rv;
	int nr;
	struct aiocb* aio = hdr->mbuf->ext.aio;

	rv = EGC_COMM_HDL_OK;
	nr = aio_return(aio);

	DEBUGF(DEBUG_LEVEL_INFO, "INFO: aio check result:%d (estimate %d bytes)\n", nr, (int)aio->aio_nbytes);

	if(nr != 0){

		DEBUGF(DEBUG_LEVEL_INFO, "INFO: aio buffer: %s\n", (char*)aio->aio_buf);

		aio->aio_offset += nr;
		aio->aio_nbytes -= nr;
		aio->aio_buf += nr;
		if(aio->aio_nbytes > 0){
			rv = EGC_COMM_HDL_AGAIN;
		}else{
			rv = EGC_COMM_HDL_OK;
		}
	}else{
		// aio_error で読み込み終了が出ていれば、文末に一意に決まる
		rv = EGC_COMM_HDL_WAIT;
	}
	EGC_POST_DBG("INTERNAL");
	return rv;
}

void PEGC_comm_file_close(p_commhdr *hdr){

}

static int SPEGC_comm_file_open(p_commhdr *hdr){
	struct aiocb *aio = hdr->mbuf->ext.aio;
	int fd;
//	sigset_t mask;

	char *fn;

	EGC_PRE_DBG("INTERNAL");

	if(hdr->addr.portfile != NULL){
		fn = hdr->addr.portfile;
	}else{
		fn = hdr->addr.name;
	}

	DEBUGF(DEBUG_LEVEL_INFO, "INFO: open file with name:%s\n", fn);
	fd = open(fn, O_RDWR | O_CREAT | O_APPEND, S_IRWXU);
	ERRCHK(fd < 0, "comm_file_setup: open");

	aio->aio_fildes = fd;
	aio->aio_sigevent.sigev_value.sival_int = aio->aio_fildes;

	EGC_POST_DBG("INTERNAL");
	return fd;
}


EGC_COMM_HDL PEGC_intracomm_adhoc_open_minfo(p_commhdr *hdr){
//	char wsbuf[MAX_EGC_KEYVALUE_SIZE];
	char* fnbase;
	int srcid, destid;
	int jobid;

	EGC_PRE_DBG("INTERNAL");

	srcid = hdr->minfo.srcid;
	destid = hdr->minfo.destid;

	if(srcid == -1){
		ERRORF("MINFO: srcid must be signed integer\n");
		ERROR_EXIT();
	}

	jobid = PEGCU_get_jobid();
	fnbase = PEGCU_get_confwithid("EGC_INTRA_HOST_DATAPATH", jobid);

	if(fnbase == NULL){
		ERRORF("Config: EGC_INTRA_HOST_DATAPATH%d is not defined.\n", jobid);
		ERROR_EXIT();
	}

	sprintf(hdr->addr.name, FMT_EGC_INTRSD, fnbase, srcid, destid);
	//hdr->addr.portfile = NULL;

	hdr->fd = SPEGC_comm_file_open(hdr);
	EGC_POST_DBG("INTERNAL");

	return EGC_COMM_HDL_OK;

}

void PEGC_comm_file_rcleanup(p_commhdr* hdr){
	struct aiocb *aio = hdr->mbuf->ext.aio;
	p_msgbuf *mbuf = hdr->mbuf;

	EGC_PRE_DBG("INTERNAL");

	//buf 頭に戻す file offset は進行するため変更無し
	aio->aio_nbytes = mbuf->size;
	aio->aio_buf = mbuf->buf;

	//次読み込み開始
	aio_read(aio);
	mbuf->status = EGC_FILE_AIO_ISSUED;
	EGC_POST_DBG("INTERNAL");
}

void PEGC_comm_file_ragain(p_commhdr* hdr){
	struct aiocb *aio = hdr->mbuf->ext.aio;
	p_msgbuf *buf = hdr->mbuf;

	EGC_PRE_DBG("INTERNAL");

	//再読込をかける。再読込座標やバッファ位置は read で適用済み
	aio_read(aio);
	buf->status = EGC_FILE_AIO_ISSUED;
	EGC_POST_DBG("INTERNAL");
}

EGC_COMM_STATE PEGC_comm_file_rwait(p_commhdr* hdr){
	struct aiocb *aio = hdr->mbuf->ext.aio;
	p_msgbuf *buf = hdr->mbuf;

	EGC_PRE_DBG("INTERNAL");

	//ウェイト解除、再読込座標やバッファ位置はウェイト前のモノを使う
	aio_read(aio);
	buf->status = EGC_FILE_AIO_ISSUED;
	EGC_POST_DBG("INTERNAL");

	return EGC_COMM_STATE_UNCHANGE;
}

/*
static void SPEGC_intracomm_file_complete_header(p_commhdr *hdr){
	struct aiocb *aio = hdr->mbuf->ext.aio;
	p_msginfo *minfo = (p_msginfo*) hdr->mbuf->buf;


	if(minfo->nbytes > aio->aio_nbytes){
		PEGC_comm_reallocbuf(&(hdr->mbuf), minfo->nbytes);
	}

	aio->aio_offset += sizeof(p_msginfo);
	aio->aio_buf = hdr->mbuf->buf + sizeof(p_msginfo);
	aio->aio_nbytes = hdr->mbuf->size;

	hdr->mbuf->status = EGC_MSG_STATE_INTRA_DATA;
	aio_read(aio);
	hdr->mbuf->status = EGC_FILE_AIO_ISSUED;
}
*/

// aio_read/write ハンドラ
// NOTICE: read/write 関数は再読み込みをするかどうかという感じにパッキングする(sock 実装時注意)
static int SPEGC_comm_aio_check(struct aiocb *aio, AIOFUNC aiofunc){
	int rv, nr;

	EGC_PRE_DBG("SINTERNAL");

	rv = EGC_COMM_HDL_OK;
	nr = aio_return(aio);

	if(nr != 0){
		aio->aio_offset += nr;
		aio->aio_nbytes -= nr;
		aio->aio_buf += nr;
		if(aio->aio_nbytes > 0){
			aiofunc(aio);
			rv = EGC_COMM_HDL_AGAIN;
		}
	}else{
		rv = EGC_COMM_HDL_WAIT;
	}

	EGC_POST_DBG("SINTERNAL");
	return rv;
}

int PEGC_comm_aio_handle(p_commhdr* hdr, AIOFUNC aiofunc,
		AIO_OK aiook, AIO_AGAIN aioagain, AIO_WAIT aiowait){
	struct aiocb *aio = hdr->mbuf->ext.aio;
//	p_msginfo *minfo = (p_msginfo*) hdr->mbuf->buf;
	int rv;
	EGC_PRE_DBG("INTERNAL");

	rv = SPEGC_comm_aio_check(aio, aiofunc);

	switch(rv){
	case EGC_COMM_HDL_OK:
		if(aiook != NULL)
			aiook(hdr);
		break;
	case EGC_COMM_HDL_AGAIN:
		if(aioagain != NULL)
			aioagain(hdr);
		break;
	case EGC_COMM_HDL_WAIT:
		if(aiowait != NULL)
			aiowait(hdr);
		break;
	default:
		break;
	}
	EGC_POST_DBG("INTERNAL");
	return rv;
}




#if 0
void PEGC_evcb_intracomm_file_read(evutil_socket_t fd, short ef, void* args){
	p_commhdr *hdr;
	p_evarg *evarg = (p_evarg*) args;
	int rv, result = 0;
	EGC_PRE_DBG("INTERNAL");
	LIST_FOR_EACH_ENTRY(hdr, intercomm, p_commhdr, hdrchain){
		rv = PEGC_intracomm_file_read(hdr);

	}
	LIST_FOR_EACH_ENTRY(hdr, intracomm, p_commhdr, hdrchain){
		rv = PEGC_intracomm_file_read(hdr);
		if(rv != EGC_COMM_HDL_WAIT){
			result = 1;
		}
	}

	if(result == 0){

	}

	EGC_POST_DBG("INTERNAL");
}
#endif


#if 0
int PEGC_comm_file_setup(p_commhdr *hdr, p_commaddr *addr){
	// obsolute
	EGC_PRE_DBG("INTERNAL");
	SPEGC_comm_file_fileopen(hdr, addr);
	EGC_POST_DBG("INTERNAL");
	return EGC_EVENT_NOTREGISTER;
}
#endif

#if 0
int PEGC_intracomm_file_read(p_commhdr *hdr){
	int status;
	int rv;
	long int size;
	int src;
	int dest;
	char* p;

	status = aio_error(hdr->ext.aio);
	switch(status){
	case EINPROGRESS:
		rv = -1;
		break;
	case ECANCELED:
		rv = -2;
		break;
	case 0:
		switch(hdr->mbuf.status){
		case EGC_MSG_STATE_HEADER:
			rv = PEGC_comm_aio_handle(hdr, aio_read, SPEGC_intracomm_file_complete_header, NULL, NULL);
			break;
		case EGC_MSG_STATE_DATA:
			rv = PEGC_comm_aio_handle(hdr, aio_read, SPEGC_intracomm_file_complete_data, NULL, NULL);
			break;
		case EGC_MSG_STATE_ONSEND:
			break;
		}
		break;
	default:
		ERROR_EXIT();
	}
	return rv;
}
#endif


#if 0
/*
static int SPEGC_intracomm_file_read_header(p_commhdr *hdr){
	struct aiocb *aio = hdr->ext.aio;
	p_msginfo *minfo = (p_msginfo*) hdr->mbuf.buf;
	int rv;

	EGC_PRE_DBG("SINTERNAL");

	rv = SPEGC_intracomm_file_read_checkreturn(aio);
	switch(rv){
	case EGC_COMM_HDL_OK:
		if(minfo->nbytes > aio->aio_nbytes){
			PEGC_comm_reallocbuf(&(hdr->mbuf), minfo->nbytes);
		}

		aio->aio_offset += sizeof(p_msginfo);
		aio->aio_buf = hdr->mbuf.buf + sizeof(p_msginfo);
		aio->aio_nbytes = hdr->mbuf.size;

		hdr->mbuf.status = EGC_MSG_STATE_INTRA_DATA;
		aio_read(aio);
		break;
	case EGC_COMM_HDL_AGAIN:
		break;
	case EGC_COMM_HDL_WAIT:
		break;
	default:
		break;
	}

	EGC_POST_DBG("SINTERNAL");
	return rv;
}
*/


/*
static int SPEGC_intracomm_file_read_data(p_commhdr *hdr){
	struct aiocb *aio = hdr->ext.aio;
	p_msginfo *minfo = (p_msginfo*) hdr->mbuf.buf;
	LIST_HEAD *wqueue;
	LIST_HOLDER *holder;
	int rv;

	EGC_PRE_DBG("SINTERNAL");

	rv = PEGC_comm_checkaio(aio, aio_read);

	switch(rv){
	case EGC_COMM_HDL_OK:
		wqueue = &(hdr->mbuf.wqueue);
		holder = &(hdr->desthdr->mtd.wmtd->wqueue);
		LIST_ADD_TAIL(wqueue, holder);
		hdr->mbuf.status = EGC_MSG_STATE_INTRA_ONSEND;
		break;
	case EGC_COMM_HDL_AGAIN:
		break;
	case EGC_COMM_HDL_WAIT:
		break;
	default:
		break;
	}
	EGC_POST_DBG("SINTERNAL");
	return rv;
}
*/

#endif

#if 0
EGC_COMM_STATE PEGC_comm_file_read_old(p_commhdr *hdr){
	int status;
	int rv;
	long int size;
	int src;
	int dest;
	char* p;

	status = aio_error(hdr->ext.aio);
	switch(status){
	case EINPROGRESS:
		rv = -1;
		break;
	case ECANCELED:
		rv = -2;
		break;
	case 0:
		switch(hdr->mbuf.status){
		case EGC_MSG_STATE_HEADER:
			rv = PEGC_comm_aio_handle(hdr, aio_read, SPEGC_intracomm_file_complete_header, NULL, NULL);
			break;
		case EGC_MSG_STATE_DATA:
			rv = PEGC_comm_aio_handle(hdr, aio_read, SPEGC_intracomm_file_complete_data, NULL, NULL);
			break;
		case EGC_MSG_STATE_ONSEND:
			break;
		}
		break;
	default:
		ERROR_EXIT();
	}
	return rv;

}

/* SPEGC_comm_file_renew_hdlstate:obsolute
 * ハンドルの読み込み結果から次のハンドル状態を決定する
 * o/EGC_COMM_STATE: 次のハンドルの状態
 * 		EGC_COMM_HDL_OK: 読み込みは正常に終了
 * 		EGC_COMM_HDL_AGAIN: 再読込を要する
 * 		EGC_COMM_HDL_WAIT: タイムアウト/ WAITが二度取得されたときにタイムアウトになる
 * i/p_commhdr *hdr: 対象とするハンドル
 * i/EGC_COMM_HDL newstate: 読み込みの結果得られた状態
 */
static EGC_COMM_HDL SPEGC_comm_file_renew_hdlstate(p_commhdr *hdr, EGC_COMM_HDL newstate){
	EGC_COMM_HDL rv;
	switch(newstate){
	case EGC_COMM_HDL_OK:
		rv = EGC_COMM_HDL_OK;
		break;
	case EGC_COMM_HDL_AGAIN:
		rv = EGC_COMM_HDL_AGAIN;
		break;
	case EGC_COMM_HDL_WAIT:
		if(hdr->state == EGC_COMM_HDL_WAIT){
			rv = EGC_COMM_HDL_WAIT;
		}else{
			rv = EGC_COMM_HDL_AGAIN;
		}
		break;
	}
	hdr->state = newstate;
	return rv;
}


#endif

