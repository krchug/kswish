/************************************************************
 * File: kswish_subsystem_start.c
 * Objective: kswish start constructs
 * Author: KC
 * Date: 12-12-2019
 *
 ************************************************************/

#include "kswish_subsystem_start.h"

pthread_t producer_thr_tid, net_producer_thr_tid;
pthread_t consumer_thr_tid, net_consumer_thr_tid;
/*******************************************************
 * Function: kswish_subsystem_start_source
 * Objective: Starts the source threads namely, producer thread,
 * net producer thread and initialzes the producer and net
 * producer queues.
 * Input: Ip address and port
 *
 *******************************************************/
int kswish_subsystem_start_source(ip_info info)
{
	int rc = EOK;

	//Init the producer queue
	syslog(KSWISH_LOG_INFO, "Initializing producer queue");
	rc = kswish_q_init(PROD_Q);
	if (rc != EOK){
		syslog(KSWISH_LOG_ERROR, "Unable to init producer queue. Exiting");
		rc = ENOENT;
		goto exit;
	}
	syslog(KSWISH_LOG_INFO, "Initialized producer queue");

	//Init the net producer queue
	syslog(KSWISH_LOG_INFO, "Initializing net producer queue");
	rc = kswish_q_init(NET_PROD_Q);
        if (rc != EOK){
                syslog(KSWISH_LOG_ERROR, "Unable to init net producer queue. Exiting");
                rc = ENOENT;
                goto exit;
        }
	syslog(KSWISH_LOG_INFO, "Initialized net producer queue");

	//Initialize the internal hash
	syslog(KSWISH_LOG_INFO, "Initializing hash");
	kswish_hash_init();
	syslog(KSWISH_LOG_INFO, "Initialized hash");

	//Start the producer thread
	syslog(KSWISH_LOG_INFO, "Starting producer thread");
	rc = pthread_create(&producer_thr_tid, NULL, producer_thr, (void *)&info);
	if (rc != 0 ) {
		syslog(KSWISH_LOG_ERROR, "Unable to create producer thread. Exiting");
                rc = ENOENT;
                goto exit;
	}
	syslog(KSWISH_LOG_INFO, "Started producer thread");
	//Start the net producer thread
	syslog(KSWISH_LOG_INFO, "Starting net producer thread");
	rc = pthread_create(&net_producer_thr_tid, NULL, net_producer_thr, (void *)&info);
	if (rc != 0 ) {
                syslog(KSWISH_LOG_ERROR, "Unable to create net producer thread. Exiting");
                rc = ENOENT;
                goto exit;
        }
	syslog(KSWISH_LOG_INFO, "Started net producer thread");


exit:
	return rc;

}
//start subsystem on the target
int kswish_subsystem_start_target(ip_info info)
{

	int rc = EOK;
	//Init the consumer queue
	syslog(KSWISH_LOG_ERROR, "Init consumer queue");
        rc = kswish_q_init(CON_Q);
        if (rc != EOK){
                syslog(KSWISH_LOG_ERROR, "Unable to init consumer queue. Exiting");
                rc = ENOENT;
                goto exit;
        }

	syslog(KSWISH_LOG_ERROR, "Inited consumer queue");
	syslog(KSWISH_LOG_ERROR, "Init net consumer queue");
        //Init the net consumer queue
        rc = kswish_q_init(NET_CON_Q);
        if (rc != EOK){
                syslog(KSWISH_LOG_ERROR, "Unable to init net consumer queue. Exiting");
                rc = ENOENT;
                goto exit;
        }
	syslog(KSWISH_LOG_ERROR, "Inited consumer queue");

	//Start the consumer thread
	syslog(KSWISH_LOG_ERROR, "start consumer thread");
	rc = pthread_create(&consumer_thr_tid, NULL, consumer_thr, (void *)&info);
	if (rc != 0 ) {
                syslog(KSWISH_LOG_ERROR, "Unable to create consumer thread. Exiting");
                rc = ENOENT;
                goto exit;
        }
	syslog(KSWISH_LOG_ERROR, "started consumer thread");
	//Start the net consumer thread
	syslog(KSWISH_LOG_ERROR, "Start net consumer thread");
	rc = pthread_create(&net_consumer_thr_tid, NULL, net_consumer_thr, (void *)&info);
	if (rc != 0 ) {
                syslog(KSWISH_LOG_ERROR, "Unable to create net consumer thread. Exiting");
                rc = ENOENT;
                goto exit;
        }
	syslog(KSWISH_LOG_ERROR, "Started net consumer thread");
exit:
	return rc;

}

/**************************************************
 * Function: kswish_subsystem_stop_source
 * Ojective: Stop the subsystem on source
 * Input: Nothing
 * Output: Return value EOK if succesful
 *
 **************************************************/
int kswish_subsystem_stop_source()
{
	int rc = EOK;
	//TODO: Work on a cleaner way to shut down

	syslog(KSWISH_LOG_ERROR, "Clearing hash");
	//Clear the hash
	rc = kswish_hash_clear();
	if (rc != EOK){
		syslog(KSWISH_LOG_ERROR, "Unable to clear hash. Exiting");
		rc = ENOENT;
		goto exit;
	}
	syslog(KSWISH_LOG_ERROR, "cleared hash");
	//Cancel the threads
	syslog(KSWISH_LOG_ERROR, "Cancel producer thread");
	rc = pthread_cancel(producer_thr_tid);
	if (rc != EOK) {
		syslog(KSWISH_LOG_ERROR, "Unable to cancel producer thread. Exiting");
                rc = ENOENT;
                goto exit;
	}
	syslog(KSWISH_LOG_ERROR, "Canceled producer thread");
	syslog(KSWISH_LOG_ERROR, "Cancel net producer thread");
	rc = pthread_cancel(net_producer_thr_tid);
        if (rc != EOK) {
                syslog(KSWISH_LOG_ERROR, "Unable to cancel net producer thread. Exiting");
                rc = ENOENT;
                goto exit;
        }
	syslog(KSWISH_LOG_ERROR, "Canceled net producer thread");
exit:
	return rc;
}

/******************************************************
 * Function: kswish_subsystem_stop_target
 * Objective: Stop the subsystem on target
 * Input: Nothing
 * Returns: return value EOK if succesful
 *
 *****************************************************/
int kswish_subsystem_stop_target()
{
	//TODO: Clean the queues if we are shutting down
	int rc = EOK;
	//Cancel the threads
	rc = pthread_cancel(consumer_thr_tid);
	if (rc != EOK) {
		syslog(KSWISH_LOG_ERROR, "Unable to cancel consumer thread. Exiting");
		rc = ENOENT;
		goto exit;
	}

	rc = pthread_cancel(net_consumer_thr_tid);
	if (rc != EOK) {
		syslog(KSWISH_LOG_ERROR, "Unable to cancel net consumer thread. Exiting");
		rc = ENOENT;
		goto exit;
	}

exit:
	return rc;

}

