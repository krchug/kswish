#include "kswish_types.h"
#include "kswish_subsystem_start.h"
#include "kswish_log.h"
#include "kswish_queue.h"
#include "kswish_master_record.h"
int main ()
{
	int rc = EOK;
	ip_info info;
//	char *ip = "127.0.0.1";

	strcpy(info.ip, "127.0.0.1");
	info.port = 8085;

	openlog ("source", LOG_PID, LOG_DAEMON);
	setlogmask (LOG_UPTO (LOG_ERR));
	rc =kswish_subsystem_start_source(info);
	if (rc != EOK){
		fprintf(stderr, "Subsystem start failed");
	}

	int i=0;
	for (i=0; i<KSWISH_MR_MAX_CNT; i++) {
		kswish_q_s *node_add = NULL;
		rc = kswish_q_node_create(&node_add);
		if (rc != EOK) {
			syslog(KSWISH_LOG_ERROR, "PROD_Q:Unable to create node for q. Exiting");
			rc = ENOENT;
		}

		memset(node_add->data, 1, 4096);
		rc = kswish_q_add(node_add, PROD_Q);
		if (rc != EOK) {
			syslog(KSWISH_LOG_ERROR, "PROD_Q:Unable add node to net prod. Exiting");
			rc = ENOENT;
		}

	}
	sleep(20);

	rc =  kswish_subsystem_stop_source();
	if (rc != EOK){
                fprintf(stderr, "Subsystem start failed");
        }

	sleep(20);
	return rc;
}
