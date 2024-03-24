/************************************************************
 * File: kswish_threads.c
 * Objective: Supporting definitions for all the threads
 * Author: KC
 * Date: 15-12-2019
 *
 *************************************************************/


#include "kswish_threads.h"
#include "kswish_queue.h"
#include "kswish_subsystem_start.h"
#include "kswish_headers.h"

static int kswish_start_server_on_consumer(ip_info *info)
{
	int sockfd, connfd, len;
	struct sockaddr_in servaddr, cli;

	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		syslog(KSWISH_LOG_ERROR,"NET_CON_THR: socket creation failed.... Exiting\n");
		connfd = -1;
                goto exit;
	}

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(8085);

	// Binding newly created socket to given IP and verification
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
		syslog(KSWISH_LOG_ERROR,"NET_CON_THR: socket bind failed...\n");
		connfd = -1;
		goto exit;
	} else {
		syslog(KSWISH_LOG_ERROR,"NET_CON_THR: Socket successfully binded..\n");
	}

	// Now server is ready to listen and verification
	if ((listen(sockfd, 5)) != 0) {
		syslog(KSWISH_LOG_ERROR,"NET_CON_THR: Listen failed...\n");
		connfd = -1;
		goto exit;
	} else {
		syslog(KSWISH_LOG_ERROR,"NET_CON_THR: Server listening..\n");
	}

	len = sizeof(cli);

	// Accept the data packet from client and verification
	connfd = accept(sockfd, (SA*)&cli, (socklen_t*)&len);
	if (connfd < 0) {
		syslog(KSWISH_LOG_ERROR,"NET_CON_THR: server acccept failed...\n");
		goto exit;
	}


exit:
	return connfd;
}
static int kswish_start_client_on_producer(ip_info *info)
{
	char *ip = info->ip;
	int port = info->port;
	//This is a bad code. Right now we wait for 20 seconds for the server to be spawned
	int retry_count = 20;
	int i = 0;

	int sockfd;
	struct sockaddr_in servaddr;

	syslog(KSWISH_LOG_ERROR, "NET_PROD_Q: Ip is %s and port is %d", ip, port);

	//Create a socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		syslog(KSWISH_LOG_ERROR,"NET_PROD_Q: socket creation failed.... Exiting\n");
		goto exit;
	} else{
		syslog(KSWISH_LOG_ERROR, "NET_PROD_Q:Socket successfully created..\n");
	}

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(8085);

	// connect the client socket to server socket
retry:
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
		syslog(KSWISH_LOG_ERROR,"NET_PROD_Q:connection with the server failed...\n");
		i ++;
		if (i < retry_count) {
			sleep(1);
			syslog(KSWISH_LOG_ERROR,"NET_PROD_Q: Retrying");
			goto retry;
		}
		sockfd = -1;
		goto exit;
	}

	syslog(KSWISH_LOG_ERROR,"NET_PROD_Q:Socket %u is ready\n", sockfd);
exit:
	return sockfd;
}
void *net_consumer_thr(void *vargp)
{
	int sockfd = 0, rc = EOK;
	//int rc = EOK;
	//int count = 0;
	ip_info *info = (ip_info*)vargp;

	syslog(KSWISH_LOG_ERROR, "KILL: In net consumer thread");
	//Start your client on the sender
	sockfd = kswish_start_server_on_consumer(info);
	if (sockfd == -1){
		syslog(KSWISH_LOG_ERROR, "NET_CON_Q:Unable to start tcp service on producer. Exiting");
		goto exit;
	}

	//Make a blocking call on the read on the sockfd
	while (1){
		syslog(KSWISH_LOG_ERROR, "NET_CON_Q:Waiting on getting metadata");
		rc = kswish_recieve_metadata(sockfd);
		if (rc != EOK){
			syslog(KSWISH_LOG_ERROR, "NET_CON_Q:Unable to recieve metadata. Exiting");
			goto exit;
		}

		rc = kswish_recieve_data(sockfd);
		if (rc != EOK){
			syslog(KSWISH_LOG_ERROR, "NET_CON_Q:Unable to recieve data. Exiting");
			goto exit;
		}

		syslog(KSWISH_LOG_ERROR, "NET_CON_Q:Signalling consumer thread now");
		//Signal the consumer thread to start working
		pthread_mutex_lock(&net_consume_queue.lock);
		pthread_cond_signal(&net_consume_queue.condition);
		pthread_mutex_unlock(&net_consume_queue.lock);
	}

exit:
	return NULL;
}
void *consumer_thr(void *vargp)
{
	int rc = EOK;
	int count = 0;
	kswish_q_s *node_add = NULL;
        kswish_q_s *tmp = NULL, *tmp1=NULL;
	kswish_master_record *master_record = NULL;

	syslog(KSWISH_LOG_ERROR, "KILL: in consumer thread");
restart:
	syslog(KSWISH_LOG_ERROR, "CON_Q: Waiting on signal from the net consumer");
	//Wait on the conditional variable of net consume queue for data
	pthread_mutex_lock(&net_consume_queue.lock);
	pthread_cond_wait(&net_consume_queue.condition, &net_consume_queue.lock);

	pthread_mutex_unlock(&net_consume_queue.lock);

	syslog(KSWISH_LOG_ERROR, "CON_Q: Signal recieved");
	//Once we are signalled read the master record first
	tmp = net_consume_queue.head;

	//read the frst block. this should be the mr
	syslog(KSWISH_LOG_ERROR, "CON_Q: Remove MR");
	rc = kswish_q_remove(NET_CON_Q, &tmp);
	if (rc != EOK){
		syslog(KSWISH_LOG_ERROR, "CON_Q:Unable to remove node from queue. Exiting");
		rc = ENOENT;
		goto exit;
	}
	master_record = (kswish_master_record *)tmp->data;

	int i = 0;
	//Loop over the master record to rebuild the data
	for (i=0; i< KSWISH_MR_MAX_CNT; i++) {
		syslog(KSWISH_LOG_ERROR, "CON_Q: master_record[i].blk_number %lu master_record[i].lba %lu",
				master_record[i].blk_number, master_record[i].lba);
		if (master_record[i].blk_number == master_record[i].lba){
			//This means that the block is original
			//LEts add this block to the consumer queue

			syslog(KSWISH_LOG_ERROR, "CON_Q: This is unique adding to consumer queue");
			rc = kswish_q_remove(NET_CON_Q, &node_add);
			if (rc != EOK){
				syslog(KSWISH_LOG_ERROR, "CON_Q:Unable to remove node from queue. Exiting");
				rc = ENOENT;
				goto exit;
			}

			rc = kswish_q_add(node_add, CON_Q);
                        if (rc != EOK) {
                                syslog(KSWISH_LOG_ERROR, "CON_Q:Unable add node to conq. Exiting");
                                rc = ENOENT;
                                goto exit;
                        }
			syslog(KSWISH_LOG_ERROR, "CON_Q: Q add success");
		} else {
			//This means the block is a duplicate so we need to take the
			//lba and create a copy of it and add it to the queue
			//blknumber is our block and it is a copy of lba
			syslog(KSWISH_LOG_ERROR, "CON_Q: This is not unique");
			tmp1 = consumer_queue.head;
			count = 0;
			while (tmp1!=NULL && count != master_record[i].lba){
				tmp1 = tmp1->next;
				count++;
			}

			if (tmp1 == NULL){
				//Comething wrong has happened here
				syslog(KSWISH_LOG_ERROR, "CON_Q: tmp1 is null tat means the block which this block is a copy is not there");
				return NULL;
			}
			if (count != master_record[i].lba){
				//Someting is wrong
				syslog(KSWISH_LOG_ERROR, "CON_Q: Same error. we dont have the block in the list");
				return NULL;
			}
			node_add = NULL;
			syslog(KSWISH_LOG_ERROR, "CON_Q: Creating new queue node");
			//Now lets add it to the consumer queue
                        rc = kswish_q_node_create(&node_add);
                        if (rc != EOK) {
                                syslog(KSWISH_LOG_ERROR, "CON_Q:Unable to create node for q. Exiting");
                                rc = ENOENT;
                                goto exit;
                        }

			syslog(KSWISH_LOG_ERROR, "CON_Q: Copy the data from the original block to recreate a block");
                        //copy the data
                        memcpy(node_add->data, tmp1->data, BLK_SIZE);

                        rc = kswish_q_add(node_add, CON_Q);
                        if (rc != EOK) {
                                syslog(KSWISH_LOG_ERROR, "CON_Q:Unable add node to CON. Exiting");
                                rc = ENOENT;
                                goto exit;
                        }
			syslog(KSWISH_LOG_ERROR, "CON_Q: Queue addition success.");
		}
	}

	//Once you are done with everything just end it.
	goto restart;

exit:
	return NULL;
}
void *net_producer_thr(void *vargp)
{
	int sockfd = 0;
	int rc = EOK;
	int count = 0;
	ip_info *info = (ip_info*)vargp;
	kswish_q_s *queue_node = NULL;
	kswish_q_s *tmp = NULL;

	syslog(KSWISH_LOG_INFO, " KILL :Started net producer thread");
	//Start your client on the sender
	sockfd = kswish_start_client_on_producer(info);
	if (sockfd == -1){
		syslog(KSWISH_LOG_ERROR, "PROD_Q:Unable to start tcp service on producer. Exiting");
                goto exit;
	}

restart:
	syslog(KSWISH_LOG_ERROR, " NET_PROD_Q:Waiting on the mutex");
	//We wait on the signal from the producer queue
	pthread_mutex_lock(&net_produce_queue.lock);
        pthread_cond_wait(&net_produce_queue.condition, &net_produce_queue.lock);
        pthread_mutex_unlock(&net_produce_queue.lock);
	syslog(KSWISH_LOG_ERROR, " NET_PROD_Q:We have been signalled. Going ahead");
	//Once the signal is recieved we start looking at the queue and start sending it on the socket
	//cache the head
	tmp = net_produce_queue.head;
	//cache the count
	count = net_produce_queue.count;
	syslog(KSWISH_LOG_ERROR, " NET_PROD_Q:Count of blocks in queue %d", count);
	//Mark the queue as done. init the queue since we have cached the head
	rc = kswish_q_init(NET_PROD_Q);
	if (rc != EOK){
		syslog(KSWISH_LOG_ERROR, "PROD_Q:Unable to init net producer queue. Exiting");
                goto exit;
	}

	syslog(KSWISH_LOG_ERROR, " NET_PROD_Q:Init of net prod q success. Sending metadata nowq");
	rc = kswish_send_metadata(tmp, sockfd);
	if (rc != EOK){
                syslog(KSWISH_LOG_ERROR, "NET_PROD_Q:Unable to send metadata. Exiting");
                goto exit;
        }
	syslog(KSWISH_LOG_ERROR, " NET_PROD_Q:MEtadata send success");
	//Cache the mr
	queue_node = tmp;
	//Move the tmp pointer ahead
	tmp = tmp->next;
	//Free the master record
	free(queue_node);
	//Decrease the count
	count -= 1;
	syslog(KSWISH_LOG_ERROR, " NET_PROD_Q:Count now is %d. Sending data now", count);
	//send the data
	rc = kswish_send_data(tmp, sockfd, count);
	if (rc != EOK) {
		syslog(KSWISH_LOG_ERROR, "PROD_Q:Unable to send data. Exiting");
		goto exit;
	}

	//once we are done sending the data over the network we go and wait again.
	goto restart;
	//TODO: think about closing the socket
exit:
	return NULL;
}
void *producer_thr(void *vargp)
{
	int rc = EOK;
	kswish_q_st* producer = &producer_queue;
	int count = 0;
	kswish_q_s *node = NULL;
	crc_pair crc;
	kswish_unique_return id;
	kswish_master_record *master_record = NULL;
	kswish_master_record_meta *metadata = NULL;
	kswish_q_s *node_add = NULL;
	//Check if you have 256 blocks to work with
	//Need to also add a timer here
	//So it will be either 256 blocks or timer

restart:
	count =0;
	producer = &producer_queue;
	syslog(KSWISH_LOG_INFO, "KILL : Producer thread started");
	printf("KRC: Producer count is %d", producer->count);
	syslog(KSWISH_LOG_INFO, "KILL : Producer count is %d", producer->count);
	while(producer->count < KSWISH_MR_MAX_CNT){
		//TODO: This is a very bad code of adding sleep. Need to find another way
		syslog(KSWISH_LOG_ERROR, " PROD_Q:Nothing to do since q is empty. Sleeping.");
		sleep(1);
	}

	syslog(KSWISH_LOG_ERROR, " PROD_Q: We have 256 blocks now so moving ahead");
	//Init the variables
	master_record = NULL;
	metadata = NULL;
	//create a master record
	syslog(KSWISH_LOG_ERROR, " PROD_Q:Creating master record");
	rc = kswish_create_master_record_block(&master_record, &metadata);
	if (rc != EOK){
		syslog(KSWISH_LOG_ERROR, "PROD_Q:Unable to create master record. Exiting");
		rc = ENOENT;
		goto exit;
	}
	syslog(KSWISH_LOG_ERROR, " PROD_Q:created master record");
	node = NULL;
	//while you process all blocks
	while (count < KSWISH_MR_MAX_CNT){
		printf("\nCount is %d\n", count);
		//Get one block from the queue
		syslog(KSWISH_LOG_ERROR, " PROD_Q: Removing node from prod q");
		syslog(KSWISH_LOG_ERROR, " PROD_Q: Removing node from prod q");
		rc = kswish_q_remove(PROD_Q, &node);
		if (rc != EOK){
			syslog(KSWISH_LOG_ERROR, "PROD_Q:Unable to remove node from queue. Exiting");
			rc = ENOENT;
			goto exit;
		}

		syslog(KSWISH_LOG_ERROR, " PROD_Q:node remove success %p", node);
		//Okay you got the block, now take the crc of the same
		rc = crc_return_from_base((const char*)node->data, BLK_SIZE, &crc);
		if (rc != EOK) {
			syslog(KSWISH_LOG_ERROR, "PROD_Q:Unable to calculate crc. Exiting");
                        rc = ENOENT;
                        goto exit;
		}
		syslog(KSWISH_LOG_ERROR, " PROD_Q:CRC DONE");
		syslog(KSWISH_LOG_ERROR, "PROD_Q:CRC is %u and reduced crc is %u", crc.crc, crc.reduced_crc);

		//Now we got the crc. Now add it to the hash and see if it is unique or not
		syslog(KSWISH_LOG_ERROR, " PROD_Q:Adding to hash");
		rc = kswish_hash_add_check(crc, count, &id);
		if (rc != EOK) {
			syslog(KSWISH_LOG_ERROR, "PROD_Q:Unable to add to hash. Exiting");
                        rc = ENOENT;
                        goto exit;
		}

		syslog(KSWISH_LOG_ERROR, "PROD_Q:pba is %lu and block uniqueness is %d", id.pba, id.unique);

		if (id.unique == true) {
			//This is a case when the block is unique so you will add it to master record
			rc = kswish_insert_in_master_record(master_record, metadata, id.pba, count);
			if (rc != EOK) {
				syslog(KSWISH_LOG_ERROR, "PROD_Q:Unable to add it to master record. Exiting");
				rc = ENOENT;
				goto exit;
			}

			//Now lets add it to the net producer queue
			syslog(KSWISH_LOG_ERROR, " PROD_Q:Creating node");
			rc = kswish_q_node_create(&node_add);
			if (rc != EOK) {
				syslog(KSWISH_LOG_ERROR, "PROD_Q:Unable to create node for q. Exiting");
				rc = ENOENT;
				goto exit;
			}
			syslog(KSWISH_LOG_ERROR, " PROD_Q:Node create success");
			//copy the data
			memcpy(node_add->data, node->data, BLK_SIZE);
			syslog(KSWISH_LOG_ERROR, " PROD_Q:Node add to prod q");
			rc = kswish_q_add(node_add, NET_PROD_Q);
			if (rc != EOK) {
                                syslog(KSWISH_LOG_ERROR, "PROD_Q:Unable add node to net prod. Exiting");
                                rc = ENOENT;
                                goto exit;
                        }
			syslog(KSWISH_LOG_ERROR, " PROD_Q:Node add to net prod q success");

		} else {
			//This is the case where the block is not unique and so we will add it to the master record only
			syslog(KSWISH_LOG_ERROR, " PROD_Q:Not unique. just making entry in master record");
			rc = kswish_insert_in_master_record(master_record, metadata, id.pba, count);
                        if (rc != EOK) {
                                syslog(KSWISH_LOG_ERROR, "PROD_Q:Unable to add it to master record. Exiting");
                                rc = ENOENT;
                                goto exit;
                        }
			syslog(KSWISH_LOG_ERROR, " PROD_Q:Entry in master record done");

		}

		//Go ahead and free the node
		free(node);

		++count;

	}

	//Init the prod q since we are done
	kswish_q_init(PROD_Q);

	syslog(KSWISH_LOG_ERROR, " PROD_Q:All blocks processed. now adding the master record at head");
	//Once all the blocks have been processed, add the masterrecord in the front of the queue
	//This is a special operation for the queue which voilates the basic principle because we
	//add the master record to the head
	node_add = NULL;
	rc = kswish_q_node_create(&node_add);
	if (rc != EOK) {
		syslog(KSWISH_LOG_ERROR, "PROD_Q:Unable to create node for q for master record. Exiting");
		rc = ENOENT;
		goto exit;
	}
	syslog(KSWISH_LOG_ERROR, " PROD_Q:Node create success");
	//copy the data
	memcpy(node_add->data, master_record, BLK_SIZE);

	syslog(KSWISH_LOG_ERROR, " PROD_Q:Adding master record");
	rc = kswish_q_add_master_record(node_add, NET_PROD_Q);
	if (rc != EOK) {
		syslog(KSWISH_LOG_ERROR, "PROD_Q:Unable add node to net prod. Exiting");
		rc = ENOENT;
		goto exit;
	}
	syslog(KSWISH_LOG_ERROR, " PROD_Q:Success. now signalling net producer");
	//Once you are done with everything signal the conditional variable on net prod queue
	pthread_mutex_lock(&net_produce_queue.lock);
	pthread_cond_signal(&net_produce_queue.condition);
	pthread_mutex_unlock(&net_produce_queue.lock);

	//Now once you have done everything go ahead and restart again.
	goto restart;

exit:
	return NULL;
}
