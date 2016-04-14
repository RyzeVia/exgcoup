/*
 * CFR_base.c
 *
 *  Created on: 2013/01/30
 *      Author: RyzeVia
 */

#define GLOBAL_DEFINITION
#include "libcfr.h"
#include "utilmacros.h"


char* CFR_getvalue(char* token, char* defvalue){
	char *rv;
	if((rv = getenv(token)) == NULL){
		rv = defvalue;
	}

	return rv;
}

int CFR_getvalue_byint(char* token, int defvalue){
	char *c;
	int rv;


	if((c = getenv(token)) == NULL){
		rv = defvalue;
	}else{
		sscanf(c, "%d", &rv);
	}

	return rv;
}


int CFR_deletevalue(char* token){
	int rv;

	rv = unsetenv(token);

	return rv;
}

int CFR_setenv_from_file(char* filename, const char* formatter){
	char key[MAX_CFR_KEYSIZE];
	char value[MAX_CFR_VALUESIZE];
	char line[MAX_CFR_LINESIZE];
	FILE *fp;

	if((fp = fopen(filename, "r")) == NULL){
		ERROR_LOCF("Configure file (%s) is not found.\n", filename);
		return CFR_FALSE;
	}

	while(feof(fp) == 0){
		if(fgets(line, MAX_CFR_LINESIZE, fp) == NULL){
			continue;
			//ERRORF("Configure file (%s) got error on reading.\n", filename);
			//ERROR_EXIT();
		}
		// permit no \n line just before EOF
		if((feof(fp) != 0) && (line[strlen(line)-1] != '\n')) {
			ERROR_LOCF("Configure file (%s) has too long line (limit all:%d(key:%d+value:%d)): %s\n",
					filename, MAX_CFR_LINESIZE, MAX_CFR_KEYSIZE, MAX_CFR_VALUESIZE,
					line);
			ERROR_EXIT();
		}
		// skip line which has # on the first
		if(line[0] == '#'){
			continue;
		}

		// search
//		int a = sscanf(line, formatter, key, value);
//		printf("line, formatter = %s, %s\n", line, formatter);
//		printf("key,value=%s,%s, rv:%d\n", key, value, a);
		switch (sscanf(line, formatter, key, value)){

		case 1:
			unsetenv(key);
			break;
		case 2:
//			printf("setenv: key,value=%s,%s\n", key, value);
			if(setenv(key, value, 1) < 0){
				if(errno == EINVAL){
					ERROR_LOCF("Configure file(%s) has '=' on key or value.\n", filename);
					ERROR_LOCF("Passing line: %s\n", line);
				}
				else if(errno == ENOMEM){
					ERROR_LOCF("Cannot allocate memory.\n");
					ERROR_EXIT();
				}
			}
			break;
		default:
			// Unformatted line (comment ?)
			//ERROR_LOCF("There are some unexpected thing.\n");
			//ERROR_EXIT();
			break;
		}

	}

	return CFR_TRUE;

}

