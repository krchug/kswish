/******************************************************
 * Filename: kswish_queue.c
 * Objective: Implement protocols queues
 * Author: KC
 * Date: 4-12-2019
 *
 ******************************************************/

#include "kswish_queue.h"

kswish_q_st producer_queue;     //Create a queue on which the user will put the blocks
kswish_q_st consumer_queue;     //Create a queue from which the target will read the blocks
kswish_q_st net_produce_queue;      //Create a queue from on which our system will produce blocks and net system will send it
kswish_q_st net_consume_queue;      //Create a queue where from the network the data is recieved

/******************************************************
 * Function: kswish_ret_q
 * Objective: Return the proper q structure
 * Input: Type of the queue
 * Output: Pointer to the header
 *
 *****************************************************/
static kswish_q_st* kswish_ret_q (kswish_q_type type)
{
        switch(type)
        {
                case PROD_Q:
                        return(&producer_queue);
                        break;
                case NET_PROD_Q:
                        return(&net_produce_queue);
                        break;
                case CON_Q:
                        return(&consumer_queue);
                        break;
                case NET_CON_Q:
                        return(&net_consume_queue);
                        break;
                default:
                        return NULL;
        }

}
/********************************************************
 * Function: kswish_q_init
 * Objective: Init the queue on the basis of type given
 * Input: Kswish_q_type type of the queue
 * Output: return EOK if successful, ENOENT if failed
 *
 ********************************************************/
int kswish_q_init(kswish_q_type type)
{
	int rc = EOK;
	kswish_q_st *tmp = NULL;

	tmp = kswish_ret_q(type);

	tmp->head = NULL;
	tmp->rear = NULL;
	tmp->count = 0;

	pthread_cond_init( &tmp->condition, NULL );
	pthread_mutex_init( &tmp->lock, NULL );

	return rc;
}

/**************************************************
 * Function: kswish_q_node_create
 * Objective: Create an empty node and return back
 * Input: Nothing
 * Output: node of structure kswish_q_s
 * 	   rc value EOK if pass ENOENT if fail
 * ************************************************/
int kswish_q_node_create(kswish_q_s **node)
{
	int rc = EOK;
	kswish_q_s *tmp = NULL;

	//Allocate the memory for the node
	tmp = (kswish_q_s*)malloc(sizeof(kswish_q_s));

	if (tmp == NULL) {
		//TODO: ADd logging structure. Maybe fail
		syslog(KSWISH_LOG_ERROR, "Unable to allocate memory to queue node. Exiting");
		rc = ENOENT;
	} else {
		tmp->next = NULL;
		memset( tmp->data, 0, BLK_SIZE);
	}

	*node = tmp;

	return rc;
}

/******************************************************
 * Function: kswish_q_add
 * Objective: Add the element to the queue specified.
 * Addition is always done at the rear of the queue
 * Input: Node to add, type of the queue to add
 * Output: EOK if passed, ENOENT if failed
 *
 ****************************************************/
int kswish_q_add(kswish_q_s *node, kswish_q_type type)
{
	int rc = EOK;
        kswish_q_st *tmp = NULL;
	tmp = kswish_ret_q(type);

	//Check if the head is NULL
	if (tmp->head == NULL ){
		//If the head is null the rear should be null. If not something is wrong
		if (tmp->rear != NULL){
			syslog(KSWISH_LOG_ERROR, "Expected rear to be null. It isnt. Exiting");
			rc = ENOENT;
			goto exit;
		}

		//if everything is fine, go ahead and add the element to the rear
		//Mark rear=head=node
		tmp->head = tmp->rear = node;
	} else {
		//IF we already have something at the rear, add the element to the rear
		tmp->rear->next = node;
		node->next = NULL;
		tmp->rear = node;
	}

	//Increase the count of nodes in the queue
	tmp->count += 1;
exit:
	return rc;

}
/******************************************************************
 * function: kswish_q_add_master_record
 * Objective: Add the master record block at the start of the q
 * Input: Node which has master record, which q to add to
 * Returns: EOK on success
 ******************************************************************/

int kswish_q_add_master_record(kswish_q_s *node, kswish_q_type type)
{
	int rc = EOK;
        kswish_q_st *tmp = NULL;
        tmp = kswish_ret_q(type);

	//Basically add the master record to the front of the queue
	node->next = tmp->head;
	tmp->head = node;

	//Increase the count of nodes in the queue
        tmp->count += 1;
	return rc;
}
/***************************************************************
 * Function: kswish_q_remove
 * Objective: Remove a node from the head and give back
 * Input: Takes the type of the queue
 * Output: The return_node and EOK on success
 *
 **************************************************************/
int kswish_q_remove(kswish_q_type type, kswish_q_s **return_node)
{
	int rc = EOK;
        kswish_q_st *tmp = NULL;
        tmp = kswish_ret_q(type);

        //Check if the head is NULL
        if (tmp->head == NULL ){
		//If the head is NULL simply return NULL and init the q
		kswish_q_init(type);
		*return_node = NULL;
	} else {
		//Copy the return node
		*return_node = tmp->head;
		//Lets point head to head next
		tmp->head = tmp->head->next;
	}

	return rc;
}
