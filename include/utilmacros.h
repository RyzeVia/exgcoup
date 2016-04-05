#ifndef __UTILMACROS_J_H_
#define __UTILMACROS_J_H_
#define UTILM_VER 2.6


#include <stdio.h>
#include <stdlib.h>
//#include <stddef.h>
//included on stdlib
#include <string.h>
#include <strings.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <limits.h>

// global value chain
#ifdef    GLOBAL_DEFINITION
#define   GLOBAL
#define   GLOBAL_VAL(v)  = (v)
#define   GLOBAL_STVAL(v...)  = {v}
#else
#define   GLOBAL	extern
#define   GLOBAL_VAL(v)
#define   GLOBAL_STVAL(v...)
#endif

#ifndef ERROR_EXIT
#define ERROR_EXIT() exit(EXIT_FAILURE)
#endif
#ifndef ERROR_LENGTH
#define ERROR_LENGTH 64
#endif

// error message chain
#define LOCSTR_SIZE	128
#define PERRORF(size, msgs...)\
	do{\
		char __errstr[size];\
		snprintf(__errstr, size, msgs);\
		perror(__errstr);\
	}while(0)
#define PERROR_LOC(msg)\
		PERRORF(strlen(msg) + LOCSTR_SIZE, "%s[%s,%d]", msg, __FILE__, __LINE__)
#define ERRCHK(exp, msg)\
	do{\
		if ((exp) > 0){\
			PERROR_LOC(msg);\
			ERROR_EXIT();\
		}\
	}while(0)
#define ERRORF(msgs...)\
	do{\
		fprintf(stderr, msgs);\
		fflush(stderr);\
	}while(0)
#define ERROR_LOC(msg)\
	do{\
		ERRORF("%s[%s,%d]%s\n", __FUNCTION__, __FILE__, __LINE__, msg);\
	}while(0)
#ifdef DISPLAY_ERROR_LOCATION
#define ERROR_LOCF(msgs...)\
	do{\
	time_t __timet;\
	struct tm *__tm;\
	char __deblocfbuf[ERROR_LENGTH];\
	snprintf(__deblocfbuf, ERROR_LENGTH, msgs);\
	time(&__timet);\
	__tm = localtime(&__timet);\
	ERRORF("%2d/%02d %2d:%02d:%02d[%s,%d]:%s",\
			__tm->tm_mon + 1, __tm->tm_mday,\
			__tm->tm_hour, __tm->tm_min, __tm->tm_sec, __FILE__, __LINE__, __deblocfbuf);\
}while(0)
#else
#define ERROR_LOCF(msgs...) ERRORF(msgs)
#endif
#define NOTIMP() ERROR_LOC("not implemented")



// debug message
// debug level default definition
#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL	100
#endif
#define DEBUG_LEVEL_MUST	0
#define DEBUG_LEVEL_FATAL	20
#define DEBUG_LEVEL_ERROR	40
#define DEBUG_LEVEL_WARNING	60
#define DEBUG_LEVEL_INFO	80
#define DEBUG_LEVEL_TRIVIAL	100

// debug topic default definition
#ifndef DEBUG_TOPIC
#define DEBUG_TOPIC	-1
#endif
#define DEBUG_TOPIC_NETWORK		60
#define DEBUG_TOPIC_PASSTHROUGH	80
#define DEBUG_TOPIC_TRIVIAL		100


#ifdef DEBUG_ENABLE

#ifndef DEBUG_LENGTH
#define DEBUG_LENGTH 64
#endif

#ifndef DEBUG_FN_LENGTH
#define DEBUG_FN_LENGTH	128
#endif
GLOBAL FILE *__fdes	GLOBAL_VAL(NULL);

#ifdef DEBUG_FILENAME
#define DEBUG_GET_FD()\
	do{\
		if (__fdes == NULL){\
			char __dbbuf[DEBUG_FN_LENGTH];\
			int pid = getpid();\
			sprintf(__dbbuf, "%s_%d", DEBUG_FILENAME, pid);\
			__fdes = fopen( __dbbuf, "w");\
			ERRCHK(__fdes == NULL, "debug file: fopen");\
		}\
	}while(0)
#else
#define DEBUG_GET_FD()	{__fdes = stderr;}
#endif

#define DEBUG_EXPR_LEVEL(level)\
	(level < DEBUG_LEVEL)

#define DEBUG_EXPR_TOPIC(topic)\
	(topic == DEBUG_TOPIC)

#define STATIC_DEBUGPRINT(expr, msgs...)\
	do{\
		DEBUG_GET_FD();\
		if (expr) fprintf(__fdes, msgs);\
	}while(0)



#define DEBUGF(level, msgs...)\
	do{\
		STATIC_DEBUGPRINT(DEBUG_EXPR_LEVEL(level), msgs);\
	}while(0)

#define DEBUGTOPF(topic, msgs...)\
	do{\
		STATIC_DEBUGPRINT(DEBUG_EXPR_TOPIC(topic), msgs);\
	}while(0)

#define DEBUG_LOC(level, msgs...)\
	do{\
		time_t timet;\
		struct tm *tm;\
		char __deblocfbuf[DEBUG_LENGTH];\
		snprintf(__deblocfbuf, DEBUG_LENGTH, msgs);\
		time(&timet);\
		tm = localtime(&timet);\
		DEBUGF(level, "%2d/%02d %2d:%02d:%02d[%s,%d]:%s\n",\
				tm->tm_mon + 1, tm->tm_mday,\
				tm->tm_hour, tm->tm_min, tm->tm_sec, __FILE__, __LINE__, __deblocfbuf);\
	}while(0)

#define DEBUGTOP_LOC(topic, msgs...)\
	do{\
		time_t timet;\
		struct tm *tm;\
		char deblocfbuf[DEBUG_LENGTH];\
		snprintf(deblocfbuf, DEBUG_LENGTH, msgs);\
		time(&timet);\
		tm = localtime(&timet);\
		DEBUGTOPF(top, "%2d/%02d %2d:%02d:%02d[%s,%d]:%s\n",\
				tm->tm_mon + 1, tm->tm_mday,\
				tm->tm_hour, tm->tm_min, tm->tm_sec, __FILE__, __LINE__, deblocfbuf);\
	}while(0)

#define DEBCHK(level, exp, msgs...)\
	do{\
		if ((exp) > 0){\
			DEBUG_LOC(level, msgs);\
		}\
	}while(0)

#define DEBTOPCHK(topic, exp, msgs...)\
	do{\
		if ((exp) > 0){\
			DEBUGTOP_LOC(topic, msgs);\
		}\
	}while(0)

#else
#define DEBUGF(level, msgs...);
#define DEBUG_LOC(level, msg);
#define DEBCHK(level, exp, msg...);
#define DEBUGTOPF(topic, msgs...);
#define DEBUGTOP_LOC(topic, msg...);
#define DEBTOPCHK(topic, exp, msg...);
#endif
// Time stamp chain
#define TIMESTAMP_LENGTH	128
#define TIMESTAMPF(fdes, msgs...) \
	do{\
		time_t _timet;\
		struct tm *_tm;\
		char __deblocfbuf[TIMESTAMP_LENGTH];\
		snprintf(__deblocfbuf, TIMESTAMP_LENGTH, msgs);\
		time(&_timet);\
		_tm = localtime(&_timet);\
		fprintf(fdes, "%2d/%02d %2d:%02d:%02d:%s",\
				_tm->tm_mon + 1, _tm->tm_mday,\
				_tm->tm_hour, _tm->tm_min, _tm->tm_sec, __deblocfbuf);\
	}while(0)

// min-max chain
#define UTIL_MAX(a, b) ((a) > (b) ? (a) : (b))
#define UTIL_MIN(a, b) ((a) < (b) ? (a) : (b))
// malloc chain
#define MALLOC(type, vp, size)\
	do{\
		vp = (type) malloc((size));\
		ERRCHK((vp) == NULL, "malloc");\
	}while(0)
#define CALLOC(type, vp, count, size)\
	do{\
		vp = (type) calloc((count), (size));\
		ERRCHK((vp) == NULL, "calloc");\
	}while(0)
#define REALLOC(type, vp, prevsize, newsize)\
	do{\
		type tp;\
		tp = (type) realloc((vp), (newsize));\
		if(tp != vp){\
			memcpy(tp, vp, UTIL_MIN(prevsize, newsize));\
			free(vp);\
		}\
		vp = tp;\
	}while(0)

// frequent use functions

/* Timespec macro */
#define TVPRINTF(tv_p) (int)((tv_p)->tv_sec), (long int)((tv_p)->tv_usec)
#define TVSCANF(tv_p) (int*)(&((tv_p)->tv_sec)), (long int*)(&((tv_p)->tv_usec))
#define TSPRINTF(tv_p) (int)((tv_p)->tv_sec), (long int)((tv_p)->tv_nsec)
#define TSSCANF(tv_p) (int*)(&((tv_p)->tv_sec)), (long int*)(&((tv_p)->tv_nsec))
#define TSCLEAR(ts_p) ((ts_p)->tv_sec = (ts_p)->tv_nsec = 0)
#define TSISSET(ts_p) ((ts_p)->tv_sec || (ts_p)->tv_nsec)
#define TSCMP(a_p, b_p, CMP) \
	(((a_p)->tv_sec == (b_p)->tv_sec) ? \
     ((a_p)->tv_nsec CMP (b_p)->tv_nsec) : \
     ((a_p)->tv_sec CMP (b_p)->tv_sec))
#define TSABS(a_p, result_p) \
	do { \
		struct timespec ___tvzero; \
		TSCLEAR(&___tvzero); \
		if (TSCMP(a_p, &___tvzero, <)) \
		{ \
			TSSUB(&___tvzero, a_p, result_p); \
		} else { \
			TSSUB(a_p, &___tvzero, result_p); \
		} \
	} while(0)
#define TSADD(a_p, b_p, result_p) \
  do { \
    (result_p)->tv_sec = (a_p)->tv_sec + (b_p)->tv_sec; \
    (result_p)->tv_nsec = (a_p)->tv_nsec + (b_p)->tv_nsec; \
    if ((result_p)->tv_nsec >= 1000000000) \
      { \
        ++(result_p)->tv_sec; \
        (result_p)->tv_nsec -= 1000000000; \
      } \
  } while (0)
#define TSSUB(a_p, b_p, result_p) \
  do { \
    (result_p)->tv_sec = (a_p)->tv_sec - (b_p)->tv_sec; \
    (result_p)->tv_nsec = (a_p)->tv_nsec - (b_p)->tv_nsec; \
    if ((result_p)->tv_nsec < 0) { \
      --(result_p)->tv_sec; \
      (result_p)->tv_nsec += 1000000000; \
    } \
  } while (0)

#define TV2DBL(tv_p) \
		((double)(tv_p)->tv_sec + ((double)(tv_p)->tv_usec / 1000. / 1000.))

#define TS2DBL(ts_p) \
		((double)(ts_p)->tv_sec + ((double)((ts_p)->tv_nsec / 1000. / 1000. / 1000.)))

// timespec/val-double conversion
#define DBL2TV(srcdbl, dsttv_p) \
	do{ \
		(dsttv_p)->tv_usec = (susrconds_t) modf (srcdbl, &((dsttv_p)->tv_sec)) * 1000. * 1000.; \
	}while(0)

#define DBL2TS(srcdbl, dstts_p) \
	do{ \
		(dstts_p)->tv_nsec = (long int) modf (srcdbl, &((dstts_p)->tv_sec)) * 1000. * 1000. * 1000.; \
	}while(0)

/* List chain */
//#include <stddef.h>

#ifndef offsetof
#define offsetof(type, member) ((int)(&((type*)0)->member))
#endif

#define CONTAINER_OF(mem_p, conttype, memname) ({\
        const typeof(((conttype *)0)->memname ) *___ptr = (mem_p);    \
        (conttype *)((char *)___ptr - offsetof(conttype, memname));})


typedef struct list_head_ LIST_HEAD;
typedef struct list_head_ LIST_HOLDER;
struct list_head_ {
	LIST_HEAD *next, *prev;
};

// initalizer
#define INIT_LIST(list_p)\
			({((list_p)->next = (list_p)->prev = (list_p));})
#define INIT_HOLDER(holder_p) INIT_LIST(holder_p)

// fundamental function
#define LIST_INSERT(newl_p, prevl_p, nextl_p) \
	do{ \
			LIST_HEAD *___prev = prevl_p;\
			LIST_HEAD *___next = nextl_p;\
			___next->prev = (newl_p);\
			(newl_p)->next = ___next;\
			(newl_p)->prev = ___prev;\
			___prev->next = (newl_p);\
	}while(0)

#define LIST_ADD(new_p, holder_p) \
	do{ \
		LIST_INSERT((new_p), (holder_p), (holder_p)->next);\
	}while(0)
#define LIST_ADD_TAIL(new_p, holder_p)\
	do{ \
		LIST_INSERT((new_p), (holder_p)->prev, (holder_p));\
	}while(0)

#define LIST_DEL(del_p) \
	do{\
		((del_p)->next)->prev = ((del_p)->prev);\
		((del_p)->prev)->next = ((del_p)->next);\
	}while(0)
#define LIST_ISEMPTY(holder_p) ((holder_p)->next == (holder_p))
#define LIST_ENTRY(mem_p, conttype, memname) CONTAINER_OF(mem_p, conttype, memname)

// loop method
#define LIST_FOR_EACH(iterl_p, holder_p) \
	for((iterl_p) = (holder)->next; (iterl_p) != (holder_p); (iterl_p) = (iterl_p)->next)
#define LIST_FOR_EACH_WITH_NEXT(iterl_p, nextl_p, holder_p)\
	for((iterl_p) = (holder)->next, (nextl_p) = (iterl_p)->next; \
	    (iterl_p) != (holder_p); \
	    (iterl_p) = (nextl_p), (nextl_p) = (iterl_p)->next)
#define LIST_FOR_EACH_ENTRY(iterc_p, holder_p, conttype, memname) \
	for((iterc_p) = LIST_ENTRY((holder_p)->next, conttype, memname); \
	     &(iterc_p)->memname != (holder_p);\
	     (iterc_p) = LIST_ENTRY((iterc_p)->memname.next, conttype, memname))
#define LIST_FOR_EACH_ENTRY_CONTINUE(iterc_p, holder_p, startc_p, conttype, memname) \
	for((iterc_p) = (startc_p); \
		&(iterc_p)->memname != (holder_p);\
		(iterc_p) = LIST_ENTRY((iterc_p)->memname.next, conttype, memname))

// reverse loop method
#define LIST_FOR_EACH_REVERSE(iterl_p, holder_p)\
	for((iterl_p) = (holder_p)->prev; (iterl_p) != (holder_p); (iterl_p) = (iterl_p)->prev)
#define LIST_FOR_EACH_REVERSE_WITH_NEXT(iterl_p, nextl_p, holder_p)\
	for((iterl_p) = (holder_p)->prev, (nextl_p) = (iterl_p)->prev; \
	    (iterl_p) != (holder_p); \
	    (iterl_p) = (nextl_p), (nextl_p) = (iterl_p)->prev)
#define LIST_FOR_EACH_REVERSE_ENTRY(iterc_p, holder_p, conttype, memname) \
	for((iterc_p) = LIST_ENTRY((holder_p)->prev, conttype, memname); \
	     &(iterc_p)->memname != (holder_p);\
	     (iterc_p) = LIST_ENTRY((iterc_p)->memname.prev, conttype, memname))
#define LIST_FOR_EACH_REVERSE_ENTRY_CONTINUE(iterc_p, holder_p, startc_p, conttype, memname) \
	for((iterc_p) = (startc_p); \
		&(iterc_p)->memname != (holder_p);\
		(iterc_p) = LIST_ENTRY((iterc_p)->memname.prev, conttype, memname))

// blocking write/read chain */
#define BWRITE(fd, buf, size) ({ \
	int _rv_ =0;\
	char* _p_ = (buf);\
	int _len_ = (size);\
	errno = -1;\
	while(_len_ > 0){ \
		_rv_ = write((fd), _p_, _len_);\
		if(_rv_ < 0){\
			ERRORF("write: errorno:%d\n", errno);\
			ERROR_EXIT();\
		}\
		if(_rv_ == 0){\
			DEBUGF(DEBUG_LEVEL_INFO, "INFO: BWRITE, 0 writes from %d\n", _len_);\
			sleep(1);\
		}\
		_len_ -= _rv_;\
		_p_ += _rv_;\
	}\
	_rv_;\
})

/* ファイル末尾は検知しない。絶対 size 読むマン */
#define BREAD(fd, buf, size) ({ \
	int __rv = 0;\
	char* __p = (buf);\
	int __len = (size);\
	errno = -1;\
	while(__len > 0){\
		__rv = read((fd), __p, __len);\
		fflush(stdout);\
		if(__rv < 0){\
			ERRORF("read: errorno:%d\n", errno);\
			ERROR_EXIT();\
		}\
		if(__rv == 0){\
			DEBUGF(DEBUG_LEVEL_INFO, "INFO: BREAD, 0 reads from %d\n", __len);\
			sleep(1);\
		}\
		__len -= __rv;\
		__p += __rv;\
	}\
	__rv;\
})



/* unit chain */
#define UNIT_KI	(1024)
#define UNIT_K	(1000)
#define UNIT_MI	(1024*1024)
#define UNIT_M	(1000*1000)
#define UNIT_GI	(1024*1024*1024)
#define UNIT_G	(1000*1000*1000)
#define UNIT_TI	(1024*1024*1024*1024)
#define UNIT_T	(1000*1000*1000*1000)
#define UNIT2DBL(unitc_p) \
	({\
		double res = 1;\
		if (strncasecmp(unitc_p, "KI", 2) == 0) {\
			res = 1024.;\
		} else if (strncasecmp(unitc_p, "K", 1) == 0) {\
			res = 1000.;\
		} else if (strncasecmp(unitc_p, "MI", 2) == 0) {\
			res = 1024. * 1024.;\
		} else if (strncasecmp(unitc_p, "M", 1) == 0) {\
			res = 1000. * 1000.;\
		} else if (strncasecmp(unitc_p, "GI", 2) == 0) {\
			res = 1024. * 1024. * 1024.;\
		} else if (strncasecmp(unitc_p, "G", 1) == 0) {\
			res = 1000. * 1000. * 1000.;\
		} else if (strncasecmp(unitc_p, "TI", 2) == 0) {\
			res = 1024. * 1024. * 1024. * 1024.;\
		} else if (strncasecmp(unitc_p, "T", 1) == 0) {\
			res = 1000. * 1000. * 1000. * 1000.;\
		}\
	};)

// measurement chain
#ifdef	MEASURE_WTIME
#define	MWTIME_PRINT(fh_p)\
	do{\
		struct timeval tv;\
		gettimeofday(&tv, NULL);\
		fprintf(fh_p, "%f\n", TV2DBL(&tv));\
	}while(0)
#else
#define MWTIME_PRINT(fh_p)

#endif

#endif /*MISCMACRO_H_*/
