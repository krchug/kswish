#include "kswish_types.h"
#include "kswish_subsystem_start.h"

int main ()
{
	int rc = EOK;
	ip_info info;
	kswish_q_st* producer = NULL;
	int i =0;
	kswish_q_s *node = NULL;
	char data[4096];
	memset(data, 1, 4096);
	//char *ip = "127.0.0.1";

	strcpy(info.ip, "127.0.0.1");
	info.port = 8085;

	openlog ("target", LOG_PID, LOG_DAEMON);
        setlogmask (LOG_UPTO (LOG_ERR));

	rc =kswish_subsystem_start_target(info);
	if (rc != EOK){
		fprintf(stderr, "Subsystem start failed");
	}

	producer = &consumer_queue;

	while (producer->count < KSWISH_MR_MAX_CNT-1){
                //TODO: This is a very bad code of adding sleep. Need to find another way
                syslog(KSWISH_LOG_ERROR, " PROD_Q:Nothing in the queue count =%d", producer->count);
                sleep(10);
        }

	syslog(KSWISH_LOG_ERROR, " PROD_Q:Count %d Moving ahead", producer->count);


	while(i<KSWISH_MR_MAX_CNT){
		syslog(KSWISH_LOG_ERROR, "Getting out block number %d", i);
		rc = kswish_q_remove(CON_Q, &node);
                if (rc != EOK){
                        syslog(KSWISH_LOG_ERROR, "PROD_Q:Unable to remove node from queue. Exiting");
                        rc = ENOENT;
                        goto exit;
                }
		rc = memcmp(&node->data, &data, 4096);
		if (rc !=0){
			syslog(KSWISH_LOG_ERROR, "memcmp failed. Exiting");
			rc = ENOENT;
		}
		syslog(KSWISH_LOG_ERROR, "memcmp Passed. Going head");
		i++;
	}

	sleep(20);

exit:
	rc =  kswish_subsystem_stop_target();
	if (rc != EOK){
                fprintf(stderr, "Subsystem start failed");
        }

	return rc;
}
