/*
 * utilaspect.h
 *
 *  Created on: 2014/10/16
 *      Author: RyzeVia
 */

#ifndef UTILASPECT_H_
#define UTILASPECT_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>


#ifdef ASP_PREPOST
#define ASP_PRE_PROC(funcname,tag) ASP_pre_procedure(funcname,tag)
#define ASP_POST_PROC(funcname,tag) ASP_post_procedure(funcname,tag)
#else
#define ASP_PRE_PROC(funcname,tag);
#define ASP_POST_PROC(funcname,tag);
#endif

#define ASP_PRINT_EACH_CONDITION(op, fn, lab, caption) \
        if((strcmp(funcname, fn) == 0) op (strcmp(tag, lab) == 0)) { \
                fprintf(stderr, "[%d]%s: %s:%s\n", (int)getpid(), caption, fn, lab); \
        }

#define ASP_PRINT_EACH_FUNCTION(fn, caption) ASP_PRINT_EACH_CONDITION(&&, fn, tag, caption)
#define ASP_PRINT_EACH_TAG(lab, caption) ASP_PRINT_EACH_CONDITION(&&, funcname, lab, caption)


void ASP_pre_procedure(const char* funcname, const char* tag);
void ASP_post_procedure(const char* funcname, const char* tag);

#endif /* UTILASPECT_H_ */
