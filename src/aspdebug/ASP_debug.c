/*
 * ASP_debug.c
 *
 *  Created on: 2014/10/16
 *      Author: RyzeVia
 */
#include "utilaspect.h"

void ASP_pre_procedure(const char* funcname, const char* tag){
//	ASP_PRINT_EACH_TAG("EXTERNAL", "EENTER:");
//	ASP_PRINT_EACH_TAG("INTERNAL", "IENTER:");
	ASP_PRINT_EACH_FUNCTION("PEGC_comm_file_read", "FEN:");
	ASP_PRINT_EACH_FUNCTION("PEGC_intracomm_file_write", "FEN:");
	ASP_PRINT_EACH_FUNCTION("PEGC_intercomm_sock_write", "FEN:");
	ASP_PRINT_EACH_FUNCTION("PEGC_intercomm_sock_read", "FEN:");
	ASP_PRINT_EACH_FUNCTION("PEGC_intracomm_cbdata", "FEN:");
}

void ASP_post_procedure(const char* funcname, const char* tag){
//	ASP_PRINT_EACH_TAG("EXTERNAL", "EEXIT:");
//	ASP_PRINT_EACH_TAG("INTERNAL", "IEXIT:");
	ASP_PRINT_EACH_FUNCTION("PEGC_comm_file_read", "FEX:");
	ASP_PRINT_EACH_FUNCTION("PEGC_intracomm_file_write", "FEX:");
	ASP_PRINT_EACH_FUNCTION("PEGC_intercomm_sock_write", "FEX:");
	ASP_PRINT_EACH_FUNCTION("PEGC_intercomm_sock_read", "FEX:");
	ASP_PRINT_EACH_FUNCTION("PEGC_intracomm_cbdata", "FEX:");
	ASP_PRINT_EACH_FUNCTION("PEGC_intercomm_cbdata", "FEX:");
}
