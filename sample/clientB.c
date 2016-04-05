/*
 * clientB.c
 *
 *  Created on: 2016/01/17
 *      Author: RyzeVia
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define DEBUG_ENABLE

#include "egclib.h"
#include "utilmacros.h"

#define INTCOUNT	1024
#define RANK	1

int main(int argc, char** argv){
	int rank = 1, i;
	int buf[INTCOUNT];
	int buf2[INTCOUNT];

//	setenv("EGC_JID", "1", 1);
	EGC_init(rank);

	TIMESTAMPF(stderr, "client B: start reading\n");
	EGC_recv(1, (char*)buf, sizeof(int)*INTCOUNT);
	TIMESTAMPF(stderr, "client B: finish reading\n");

	for(i = 0; i < INTCOUNT; i++){
		printf("%d ", buf[i]);
	}

	TIMESTAMPF(stderr, "client B: second reading\n");
	EGC_recv(1, (char*)buf2, sizeof(int)*INTCOUNT);

	for(i = 0; i < INTCOUNT; i++){
		printf("%d ", buf2[i]);
	}

	return EXIT_SUCCESS;

}

