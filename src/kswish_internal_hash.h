/*********************************************************************
 * Filename: kswish_internal_hash.h
 * Objective: Implement headers for internal hash
 * Author: KC
 * Date: 05-12-2019
 *
 *********************************************************************/
#ifndef KSWISH_INT_H
#define KSWISH_INT_H

#include "kswish_types.h"
#include "kswish_crc.h"

typedef struct kswish_internal_hash {
	uint64_t hash;				//THe complete hash
	uint64_t pba;				//Physical block number in the queue
	struct kswish_internal_hash *next;	//Next pointer to enable chaining
} kswish_internal_hash;

#define MAX_HASH_BUCKET		256

typedef struct kswish_unique_return {
	bool		unique;			//IF true the block is unique and added to hash, if False, its not unique and we return the pba
	uint64_t 	pba;			//This is valid only when the block is not unique, else its 0
} kswish_unique_return;


//Init hash
void kswish_hash_init();
//Hash add
int kswish_hash_add_check(crc_pair pair, uint64_t pba, kswish_unique_return *id);
//Hash delete or clear
int kswish_hash_clear();

#endif
