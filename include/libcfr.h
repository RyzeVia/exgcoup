/*
 * libcfr.h
 *
 *  Created on: 2013/01/30
 *      Author: RyzeVia
 */

#ifndef LIBCFR_H_
#define LIBCFR_H_

#include <stdio.h>
#include <string.h>
#include <errno.h>

#define CFR_TRUE	1
#define CFR_FALSE	0

#define MAX_CFR_LINESIZE	80
#define MAX_CFR_KEYSIZE		32
#define MAX_CFR_VALUESIZE	(MAX_CFR_LINESIZE-MAX_CFR_KEYSIZE)

/*
typedef struct CFR_info_ {
	char* filename;
	FILE fp;
} CFR_info_t;

GLOBAL CFR_info_t dcfh;
*/

char* CFR_getvalue(char* token, char* defvalue);
int CFR_getvalue_byint(char* token, int defvalue);
int CFR_setenv_from_file(char* filename, const char* formatter);
int CFR_deletevalue(char* token);

#endif /* LIBCFR_H_ */
