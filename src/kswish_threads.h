/************************************************************
 * File: kswish_threads.h
 * Objective: Supporting includes for all the threads
 * Author: KC
 * Date: 13-12-2019
 *
 *************************************************************/
#ifndef KSWISH_THR
#define KSWISH_THR

#include "kswish_types.h"
#include "kswish_log.h"
#include "kswish_crc.h"
#include "kswish_internal_hash.h"
#include "kswish_master_record.h"
#define SA struct sockaddr

void *net_consumer_thr(void *vargp);
void *consumer_thr(void *vargp);
void *net_producer_thr(void *vargp);
void *producer_thr(void *vargp);

#endif
