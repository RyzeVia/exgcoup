/*
 * EGC_event.c
 *
 *  Created on: 2015/05/20
 *      Author: RyzeVia
 */


#include "egc_sys.h"
#include <getopt.h>


void PEGC_event_init(){
//	event_init();
	assert(0);
	// event_base_new に含まれているため、単独利用の必要がない
}

p_eventhdr* PEGC_event_newhdr(){
	p_eventhdr* hdr;

	EGC_PRE_DBG("INTERNAL");

	MALLOC(p_eventhdr*, hdr, sizeof(p_eventhdr));
	hdr->eb = event_base_new();

	EGC_POST_DBG("INTERNAL");
	return hdr;

}

void PEGC_event_freehdr(p_eventhdr* hdr){
	EGC_PRE_DBG("INTERNAL");

	event_base_free(hdr->eb);
	free(hdr);

	EGC_POST_DBG("INTERNAL");
}

void PEGC_event_register(p_eventhdr *ehdr, p_commhdr *lhdr, short int evflag, event_callback_fn fn){
	p_evarg *args;
	EGC_PRE_DBG("INTERNAL");

	MALLOC(p_evarg*, args, sizeof(p_evarg));
	args->hdr = lhdr;
	lhdr->ehdr = ehdr;
	lhdr->evflag = evflag;

	DEBUGF(DEBUG_LEVEL_INFO, "INFO: Watch event on socket %d\n", lhdr->fd);
	lhdr->ev = event_new(ehdr->eb, lhdr->fd, lhdr->evflag, fn, args);
	if(lhdr->to.use >= 0){
		lhdr->toev = evtimer_new(ehdr->eb, PEGC_comm_cbwaittimer, args);
	}
	event_add(lhdr->ev, NULL);
	EGC_POST_DBG("INTERNAL");
}

void PEGC_event_register_again(p_commhdr *lhdr){
	EGC_PRE_DBG("INTERNAL");
	if(lhdr->ev != NULL){
		event_add(lhdr->ev, NULL);
	}else{
		ERRORF("(TYPE %d): This hdr doesnot have event.", lhdr->mode);
		ERROR_EXIT();
	}
	EGC_POST_DBG("INTERNAL");
}

void PEGC_event_register_timer(p_commhdr *lhdr){
	EGC_PRE_DBG("INTERNAL");
	if(lhdr->to.use >= 0){
		event_add(lhdr->toev, &(lhdr->to.tv));
	}
	EGC_POST_DBG("INTERNAL");
}

p_commhdr* PEGC_event_getcommhdr(void* args){
	p_evarg *earg = (p_evarg*) args;
	p_commhdr *hdr = earg->hdr;
	return hdr;
}

void PEGC_event_loop(p_eventhdr *ehdr){
	EGC_PRE_DBG("INTERNAL");
	event_base_loop(ehdr->eb, 0);
	EGC_POST_DBG("INTERNAL");
}



