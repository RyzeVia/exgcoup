/*
 * clientA.c
 *
 *  Created on: 2016/01/16
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
	int rank = 1;
	int buf[INTCOUNT];

	int i;
	for(i = 0; i < INTCOUNT; i++){
		buf[i] = i;
	}

	setenv("EGC_JID", "0", 1);
	EGC_init(rank);

	/* test code */
	//fwrite(buf, 9600, 1, stderr);



	TIMESTAMPF(stderr, "start sending\n");

	EGC_send(1, (char*)buf, sizeof(int)*INTCOUNT);

	TIMESTAMPF(stderr, "first sending\n");

	EGC_send(1, (char*)buf, sizeof(int)*INTCOUNT);

	TIMESTAMPF(stderr, "finish sending\n");
	return EXIT_SUCCESS;

}

