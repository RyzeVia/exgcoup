/*
 * clientMultiB.c
 *
 *  Created on: 2016/07/04
 *      Author: RyzeVia
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define DEBUG_ENABLE

#include "egclib.h"
#include "utilmacros.h"

#define BCOUNT  (1024*1024*1024)
#define RANK    1

int main(int argc, char** argv){
        int rank = 1, i;
        char* buf;
        char* buf2;


        MALLOC(char*, buf, BCOUNT);
        MALLOC(char*, buf2, BCOUNT);

additional:;
	int pid = fork();
	if(pid == 0){
		rank = 1;
		np++;
		if(np != count) goto additional;
	}else{
		rank = 1 + np;
	}



//      setenv("EGC_JID", "1", 1);
        EGC_init(rank);

        TIMESTAMPF(stderr, "client B: start reading%d\n", rank);
        EGC_recv(rank, buf, BCOUNT);
        TIMESTAMPF(stderr, "client B: finish reading%d\n", rank);

//      for(i = 0; i < INTCOUNT; i++){
//              printf("%d ", buf[i]);
//      }

        EGC_recv(rank, buf2, BCOUNT);

        TIMESTAMPF(stderr, "client B: second reading%d\n", rank);

        if(pid != 0){
        	for(i = 1; i < count; i++){wait();}
        }
        return EXIT_SUCCESS;

}

