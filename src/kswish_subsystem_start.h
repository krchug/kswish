/************************************************************
 * File: kswish_subsystem_start.h
 * Objective: kswish start constructs
 * Author: KC
 * Date: 12-12-2019
 *
 ************************************************************/

#ifndef KSWISH_SS
#define KSWISH_SS

#include "kswish_types.h"
#include "kswish_queue.h"
#include "kswish_internal_hash.h"
#include "kswish_log.h"
#include "kswish_threads.h"

typedef struct ip_info {
	char ip[16];
	int port;
}ip_info;

//start subsystem on the source
int kswish_subsystem_start_source(ip_info info);
//start subsystem on the target
int kswish_subsystem_start_target(ip_info info);
//stop subsystem on the source
int kswish_subsystem_stop_source();
//stop subsystem on the target
int kswish_subsystem_stop_target();

#endif
