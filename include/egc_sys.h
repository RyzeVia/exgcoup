/*
 * egc_sys.h
 *
 *  Created on: 2014/10/10
 *      Author: RyzeVia
 */

#ifndef EGC_SYS_H_
#define EGC_SYS_H_

//#define ASP_PREPOST
//#define DEBUG_ENABLE
/*
#ifndef DEBUG_FILENAME
#define DEBUG_FILENAME	"/work/log/egcpops"
#endif
*/

#define DEBUG_SSH_OUTPUT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/timerfd.h>
#include <sys/signalfd.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>
#include <aio.h>


/* Incompatible headers */
/* FIXME(LOW): プリプロセッサレベルでの非互換ヘッダ選択実装 */
#include <unistd.h>
#include <termios.h>


#include <event2/event.h>
#include <event2/event_struct.h>

#include "utilmacros.h"

#include "utilaspect.h"
#include "libcfr.h"
#include "procmng.h"

/* adhoc solution */
#ifndef SIGEV_SIGNAL
#define SIGEV_SIGNAL	(0)
#endif


/* default value */
#define DEF_EGC_CONFIG	"egc.conf"
#define DEF_LOCALHOST_IP	"127.0.0.1"
#define DEF_AIO_SIG_LISTEN	(SIGRTMIN+1)
#define DEF_AIO_SIG_DATA	(SIGRTMIN+2)
//#define DEF_AIO_SIG_DATA2	(SIGRTMIN+3)
//#define DEF_EGC_INTERBUF_SIZ	(4*1024*1024*1024) // 4MB
#define DEF_EGC_INTERBUF_TOSEC	(3)
//#define DEF_EGC_INTRABUF_SIZ	(4*1024*1024*1024) // 4MB
#define DEF_EGC_INTRABUF_TOSEC	(3)
#define DEF_EGC_PACKET_SIZE		(1400) // MTU1500? 固定長パケットなので小さめ
#define DEF_EGC_TOSEC			(3)
#define DEF_EGC_LISTENPORT	(37510)
#define DEF_EGC_PXYPORT	(37511) // proxy port is listen+1
#define DEF_EGC_SSHPORT	(22)
#define DEF_EGC_INTRALMODE "file"
#define DEF_EGC_INTERLMODE "socket"
#define MAX_EGC_HOSTLEN	(256)
#define MAX_EGC_FILELEN (256)
#define MAX_EGC_PASSLEN	(256)
#define MAX_EGC_UIDLEN	(64)
#define MAX_EGC_HOSTNUM	(10)
#define MAX_EGC_GREETING_PKT	(256)
#define MAX_EGC_KEYVALUE_SIZE	(256)

/* formatter */
#define FMT_EGC_CONFIG	"%[^=\n]=%[^\n]"
//#define FMT_EGC_CMDEXEC	"cd %s; %s -p %d -i %d -c %s"
#define FMT_EGC_CMDEXEC	"%s -p %d -i %d -c %s"
#define FMT_EGC_INTRSD	"%s.%d_%d"
#define FMT_EGC_INTRS	"%s.%d"
//#define FMT_EGC_CMDEXEC	"env"

/* const flags */
#define EGC_FLISTEN_BUF	(256)

#define EGC_INPUT_TERMON	(1)
#define EGC_INPUT_TERMOFF	(0)

#define EGC_ID_ANY (-1)

typedef enum EGC_COMM_HDL_ {
	EGC_COMM_HDL_OK,
	EGC_COMM_HDL_INPROGRESS,
	EGC_COMM_HDL_AGAIN,
	EGC_COMM_HDL_WAIT,
	EGC_COMM_HDL_ERROR,
	MAX_EGC_COMM_HDL
} EGC_COMM_HDL;

typedef enum EGC_COMM_STATE_ {
	// for accept
	EGC_COMM_STATE_ESTABLISH,
	EGC_COMM_STATE_UNCHANGE,

	EGC_COMM_STATE_READY,
	EGC_COMM_STATE_TIMEOUT,
	EGC_COMM_STATE_AGAIN,

	EGC_COMM_STATE_NULL,
	MAX_EGC_COMM_STATE,
} EGC_COMM_STATE;


typedef enum EGC_COMM_MODE_ {
	EGC_COMM_MODE_SOCKET,
	EGC_COMM_MODE_FILE,
	MAX_EGC_COMM_MODE,
} EGC_COMM_MODE;

typedef enum EGC_MSG_STATE_ {
	EGC_MSG_STATE_NOWORK,
	EGC_MSG_STATE_HEADER,
	EGC_MSG_STATE_DATA,
	EGC_MSG_STATE_ONSEND,
	EGC_MSG_STATE_FINISH,
/*
	EGC_MSG_STATE_INTRA_NOWORK,
	EGC_MSG_STATE_INTRA_HEADER,
	EGC_MSG_STATE_INTRA_DATA,
	EGC_MSG_STATE_INTRA_ONSEND,
	EGC_MSG_STATE_INTRA_FINISH,

	EGC_MSG_STATE_INTRA_LISTENER_NOWORK,
	EGC_MSG_STATE_INTRA_LISTENER_DATA,
	EGC_MSG_STATE_INTRA_LISTENER_FINISH,

	EGC_MSG_STATE_INTER_NOWORK,
	EGC_MSG_STATE_INTER_ONSEND,
	EGC_MSG_STATE_INTER_FINISH,
*/
	MAX_EGC_MSG_STATE,
} EGC_MSG_STATE;

#define EGC_ROPT_NULL	0x00
#define EGC_ROPT_STRING	0x01

#define EGC_MYJOBID	-1



/* macros */
#define EGC_PRE_DBG(interface_mode) \
		do{ \
			ASP_PRE_PROC(__FUNCTION__, __FILE__); \
			ASP_PRE_PROC(__FUNCTION__, interface_mode);\
		}while(0)
#define EGC_POST_DBG(interface_mode) \
		do{ \
			ASP_POST_PROC(__FUNCTION__, interface_mode); \
			ASP_POST_PROC(__FUNCTION__, __FILE__); \
		}while(0)

/* structure declaration */
typedef struct commcall_ p_commcall;
typedef struct totimer_ p_totimer;
typedef struct msginfo_ p_msginfo;
typedef struct msgbuf_ p_msgbuf;
typedef struct commaddr_ p_commaddr;
typedef struct evhdr_ p_eventhdr;
typedef struct commhdr_ p_commhdr;
typedef struct evarg_ p_evarg;
typedef struct commhost_ p_commhost;

/* structure definition */
struct commcall_ {
	int type;
	EGC_COMM_STATE (*listen)(struct commhdr_* hdr);
	EGC_COMM_STATE (*accept)(p_commhdr* newhdr, p_commhdr* lhdr);

	EGC_COMM_HDL (*intra_read)(p_commhdr* hdr);
	EGC_COMM_HDL (*intra_write)(p_commhdr* dsthdr, p_commhdr* hdr);

	EGC_COMM_HDL (*inter_read)(p_commhdr* hdr);
	EGC_COMM_HDL  (*inter_write)(p_commhdr* dsthdr, p_commhdr* hdr);

	EGC_COMM_STATE  (*connect)(p_commhdr* hdr);
	EGC_COMM_STATE  (*disconnect)();

	p_msgbuf* (*bufalloc)(int size);
	void (*buffree)(p_msgbuf* buf);

	void (*rcleanup)(p_commhdr *hdr);
	void (*ragain)(p_commhdr *hdr);
	EGC_COMM_STATE (*rwait)(p_commhdr* hdr);

	p_commhdr* (*intra_nexthdr)(p_commhdr* hdr); // for multiple hdr

	EGC_COMM_HDL (*adhoc_open_minfo)(p_commhdr* hdr);

	/* and others */
};

struct totimer_ {
	int use;
	struct timeval tv;
};

struct msginfo_ {
	int32_t srcid;
	int32_t destid;
	int32_t messageid;
	int32_t flags;
	int32_t nbytes;
};

struct siob {
	int size;
	int offset;
};

struct msgbuf_ {
	LIST_HEAD wqueue;
	char * buf;
	int size;
	int status;
	volatile union data_ {
		void* v;
		struct aiocb *aio;
		struct siob *sio;
	} ext;
};

struct commaddr_ {
	char name[MAX_EGC_HOSTLEN]; /* IP or file(listening) */
	union {
		int portnum; /* port number */
		char* portfile; /* file(data transfering) */
	};
};

struct evhdr_ {
	struct event_base *eb;
};

struct commhdr_ {
	LIST_HEAD hdrchain; //ハンドラセットかどうか
	int mode;
	int fd;

	p_commcall *call;
	p_msgbuf *mbuf;

	p_totimer to;
	p_commaddr addr;
	p_msginfo minfo;

	EGC_COMM_HDL state;

	short int evflag;
	struct event *ev; // only set from PEGC_event_XXX
	struct event *toev; // only set from PEGC_event_XXX
	p_eventhdr *ehdr; // only set from PEGC_event_XXX
};

struct evarg_ {
	union{
		LIST_HOLDER *hdrchain;
		p_commhdr *hdr;
	};
};

struct commhost_ {
	p_commhdr *hdr;
	int sshport;
	int proxyport;
	char uid[MAX_EGC_UIDLEN];
	char pass[MAX_EGC_PASSLEN];
};

typedef int(*AIOFUNC)(struct aiocb *aio);
typedef void(*AIO_OK)(p_commhdr *hdr);
typedef void(*AIO_AGAIN)(p_commhdr *hdr);
typedef void(*AIO_WAIT)(p_commhdr *hdr);

/* globale value */
GLOBAL pid_t rshpid GLOBAL_VAL(-1);
GLOBAL char* cmdname;
GLOBAL int listener_port GLOBAL_VAL(DEF_EGC_LISTENPORT);
GLOBAL p_commcall commtable[MAX_EGC_COMM_MODE];
GLOBAL LIST_HOLDER intercomm; //hdrchain
GLOBAL LIST_HOLDER intracomm; //hdrchain
GLOBAL LIST_HOLDER intra_bpool; //bufpool
GLOBAL p_commhost localhost;
GLOBAL p_commhost* hosts;
GLOBAL int aio_listen_signal GLOBAL_VAL(-1);
GLOBAL int aio_data_signal GLOBAL_VAL(-1);
GLOBAL int hostid GLOBAL_VAL(0);
//GLOBAL int intra_bsize GLOBAL_VAL(DEF_EGC_INTRABUF_SIZ);
//GLOBAL int inter_bsize GLOBAL_VAL(DEF_EGC_INTERBUF_SIZ);
//GLOBAL int aio_data2_signal GLOBAL_VAL(-1);

/* internal function */

/* EGC_comm.c */
void PEGC_comm_init();
void PEGC_comm_bufalloc(p_msgbuf *mbuf, int reserved_bufsize);
void PEGC_comm_bufrealloc(p_msgbuf *mbuf, int reserved_bufsize);
p_commhdr* PEGC_comm_newhdr_blank(EGC_COMM_MODE mode);
p_commhdr* PEGC_comm_newhdr(EGC_COMM_MODE mode, int reserved_bufsize, int tosec);
void PEGC_comm_freehdr(p_commhdr* hdr);
void PEGC_comm_addhdrchain(p_commhdr* hdr, LIST_HOLDER* holder);
p_commhdr* PEGC_comm_getnextchain(p_commhdr* hdr);
void PEGC_comm_set_bursterror_timer(p_commhdr* hdr, struct timeval *tv);
void PEGC_comm_clear_bursterror_timer(p_commhdr* hdr);
p_commhdr* PEGC_intercomm_gethdr(p_commhdr *from_hdr);
p_commhdr* PEGC_intracomm_gethdr(p_commhdr *readhdr);
p_commhdr* PEGC_comm_gethdr_w_uid(int src, int dest, LIST_HOLDER* commlist);
void PEGC_intercomm_cblistener(evutil_socket_t fd, short ef, void* args);
void PEGC_intercomm_cbdata(evutil_socket_t fd, short ef, void* args);
void PEGC_intracomm_cblistener(evutil_socket_t fd, short ef, void* args);
void PEGC_intracomm_cbdata(evutil_socket_t fd, short ef, void* args);
void PEGC_comm_cbwaittimer(evutil_socket_t fd, short ef, void* args);
EGC_COMM_MODE PEGC_intracomm_getmode(int jid);
EGC_COMM_MODE PEGC_intercomm_getmode(int jid);

/* EGC_event.c */
void PEGC_event_init();
p_eventhdr* PEGC_event_newhdr();
void PEGC_event_freehdr(p_eventhdr* hdr);
void PEGC_event_register(p_eventhdr *ehdr,p_commhdr *lhdr, short int evflag, event_callback_fn fn);
void PEGC_event_register_again(p_commhdr *lhdr);
void PEGC_event_register_timer(p_commhdr *lhdr);
p_commhdr* PEGC_event_getcommhdr(void* args);
void PEGC_event_loop(p_eventhdr* ehdr);

/* EGC_utils.c */
int PEGCU_invoke_sshtunnel_R(char* hostname, int sshport, int beyondport, int hereport, char* cmdline, char* uid, char* pass);
int PEGCU_invoke_sshtunnel_L(char* hostname, char* proxyname, int sshport, int beyondport, int hereport, char* cmdline, char* uid, char* pass);
int PEGCU_get_jobid();
char* PEGCU_get_confpath();
char* PEGCU_get_confwithid(char* token, int id);
void PEGCU_ntohl_minfo(p_msginfo* newinfo, p_msginfo* info);
void PEGCU_htonl_minfo(p_msginfo* newinfo, p_msginfo* info);

/* communication/EGC_comm_sock.c */
int PEGC_comm_sock_init(p_commcall* call);
p_msgbuf* PEGC_comm_sock_bufalloc(int bufsize);
void PEGC_comm_sock_buffree(p_msgbuf* buf);
EGC_COMM_STATE PEGC_comm_sock_listen(p_commhdr *hdr);
EGC_COMM_STATE PEGC_comm_sock_accept(p_commhdr *nhdr, p_commhdr *hdr);
EGC_COMM_HDL PEGC_intercomm_sock_read(p_commhdr* hdr);
EGC_COMM_HDL PEGC_intercomm_sock_write(p_commhdr *dsthdr, p_commhdr *hdr);
EGC_COMM_HDL PEGC_intracomm_sock_read(p_commhdr* hdr);
EGC_COMM_HDL PEGC_intracomm_sock_write(p_commhdr* dsthdr, p_commhdr* hdr);

/* communication/EGC_comm_file.c */
int PEGC_comm_file_init(p_commcall* call);
p_commhdr* PEGC_intracomm_file_nexthdr(p_commhdr* hdr);
p_msgbuf* PEGC_comm_file_bufalloc(int bufsize);
void PEGC_comm_file_buffree(p_msgbuf* buf);
EGC_COMM_STATE PEGC_comm_file_listen(p_commhdr *hdr);
EGC_COMM_STATE PEGC_comm_file_connect(p_commhdr *hdr);
EGC_COMM_STATE PEGC_comm_file_accept(p_commhdr *nhdr, p_commhdr *hdr);
EGC_COMM_HDL PEGC_comm_file_read(p_commhdr *hdr);
EGC_COMM_HDL PEGC_intracomm_file_write(p_commhdr* dsthdr, p_commhdr* hdr);
void PEGC_comm_file_close(p_commhdr *hdr);
//int SPEGC_comm_file_open(p_commhdr *hdr);
EGC_COMM_HDL PEGC_intracomm_adhoc_open_minfo(p_commhdr *hdr);
void PEGC_comm_file_rcleanup(p_commhdr* hdr);
void PEGC_comm_file_ragain(p_commhdr* hdr);
EGC_COMM_STATE PEGC_comm_file_rwait(p_commhdr* hdr);
void PEGC_evcb_intracomm_file_read(evutil_socket_t fd, short ef, void* args);
int PEGC_comm_file_setup(p_commhdr *hdr, p_commaddr *addr);
int PEGC_intracomm_file_read(p_commhdr *hdr);
EGC_COMM_STATE PEGC_comm_file_read_old(p_commhdr *hdr);

/* communication/EGC_comm_sock.c */
int PEGC_comm_sock_init(p_commcall* call);
p_msgbuf* PEGC_comm_sock_bufalloc(int bufsize);
void PEGC_comm_sock_buffree(p_msgbuf* buf);
EGC_COMM_STATE PEGC_comm_sock_listen(p_commhdr *hdr);
EGC_COMM_STATE PEGC_comm_sock_accept(p_commhdr *nhdr, p_commhdr *hdr);
EGC_COMM_STATE PEGC_comm_sock_connect(p_commhdr *hdr);
EGC_COMM_HDL PEGC_intercomm_sock_read(p_commhdr* hdr);
EGC_COMM_HDL PEGC_intercomm_sock_write(p_commhdr *dsthdr, p_commhdr *hdr);
EGC_COMM_HDL PEGC_intracomm_sock_read(p_commhdr* hdr);
EGC_COMM_HDL PEGC_intracomm_sock_write(p_commhdr* dsthdr, p_commhdr* hdr);



#endif /* EGC_SYS_H_ */
