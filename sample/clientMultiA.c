/*
 * clientMultiA.c
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
        int rank = 1;
        char *buf;

//      int i;
//      for(i = 0; i < INTCOUNT; i++){
//              buf[i] = i;
//      }

        MALLOC(char*, buf, BCOUNT);
        setenv("EGC_JID", "0", 1);

        int pid = fork();
        if(pid == 0){
                rank = 1;
        }else{
                rank = 2;
        }

        EGC_init(rank);

        /* test code */
        //fwrite(buf, 9600, 1, stderr);



        TIMESTAMPF(stderr, "start sending%d\n", rank);

        EGC_send(rank, buf, BCOUNT);

        TIMESTAMPF(stderr, "first sending\n%d", rank);

        EGC_send(rank, buf, BCOUNT);

        TIMESTAMPF(stderr, "finish sending\n%d", rank);

        if(pid != 0){ waitpid(pid);}
        return EXIT_SUCCESS;

}



