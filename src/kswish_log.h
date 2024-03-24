/**********************************************************
 * Filename: kswish_log.h
 * Objective: Implement constructs for logging
 * Author: KC
 * Date: 6-12-2019
 *
 **********************************************************/
#ifndef KSWISH_LOG
#define KSWISH_LOG

#include "kswish_types.h"
#include <syslog.h>

#define KSWISH_LOG_INFO		LOG_INFO
#define KSWISH_LOG_ERROR	LOG_ERR
//void kswishlog(int priority, char* filename, int line, char *fmt,...);
//#define KSWISHLOG(priority, char *fmt,...) syslog(priority, char *fmt,...)
#endif
