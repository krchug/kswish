/**********************************************************
 * Filename: kswish_queue.h
 * Objective: Implement constructs for the various queues
 * Author: KC
 * Date: 4-12-2019
 * **********************************************************/

#ifndef KSWISH_Q
#define KSWISH_Q

#include "kswish_types.h"
#include "kswish_log.h"

typedef struct kswish_q_s {
	char data[4096];	//Block
	struct kswish_q_s *next; //Next pointer
} kswish_q_s;

typedef struct kswish_q_st {
	struct kswish_q_s *head;	//For each queue maintain the head
	struct kswish_q_s *rear;	//For each queue maintain the rear
	uint32_t 	  count;	//Count of elements in the queue
	pthread_cond_t	  condition;	//Conditional variable guarding the q
	pthread_mutex_t	  lock;		//Lock guarding the conditional variable
} kswish_q_st;

typedef enum kswish_q_type {
	PROD_Q		= 	1,
	NET_PROD_Q	=	2,
	CON_Q		=	3,
	NET_CON_Q	=	4,
}kswish_q_type;


extern kswish_q_st producer_queue;     //Create a queue on which the user will put the blocks
extern kswish_q_st consumer_queue;     //Create a queue from which the target will read the blocks
extern kswish_q_st net_produce_queue;      //Create a queue from on which our system will produce blocks and net system will send it
extern kswish_q_st net_consume_queue;      //Create a queue where from the network the data is recieved

//Init the queues
int kswish_q_init(kswish_q_type type);
//Create a node of the q and return
int kswish_q_node_create(kswish_q_s **node);
//Add to the queue. Addition is always at the rear
int kswish_q_add(kswish_q_s *node, kswish_q_type type);
//Remove an element from the queue. Removal happens at head
int kswish_q_remove(kswish_q_type type, kswish_q_s **return_node);
//Special function to add master record to queue
int kswish_q_add_master_record(kswish_q_s *node, kswish_q_type type);

#endif
