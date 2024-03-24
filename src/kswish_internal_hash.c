/*********************************************************************
 * Filename: kswish_internal_hash.c
 * Objective: Implement functions for internal hash
 * Author: KC
 * Date: 05-12-2019
 *
 *********************************************************************/

#include "kswish_internal_hash.h"
//Since we have moded our crc by 256, we can have 256 buckets and then add the elements in there
kswish_internal_hash    internal_hash[MAX_HASH_BUCKET];
/**************************************************
 * Function: kswish_hash_init
 * Objective: Initialize the hash
 * Input: Nothing
 * Output: Nothing
 *
 **************************************************/
void kswish_hash_init()
{
	int i = 0;
	kswish_internal_hash *tmp = NULL;

	//Init all the buckets
	for (i = 0; i < MAX_HASH_BUCKET; i++) {
		tmp = &internal_hash[i];
		tmp->hash = 0;
		tmp->pba = 0;
		tmp->next = NULL;
	}
}

/*****************************************************
 * Function: kswish_hash_add_check
 * Objective: Add an element in the hash
 * Input: Crc, crc reduced, pba
 * Output: EOK if success ENOENT if failure, also if
 * unique or not. If not unique the pba which it is
 * copy of
 *****************************************************/
int kswish_hash_add_check(crc_pair pair, uint64_t pba, kswish_unique_return *id)
{
	int rc = EOK;
	int bucket = 0;
	kswish_internal_hash *tmp = NULL;
	kswish_internal_hash *node = NULL;

	//Our bucket will be the reduced crc
	bucket = pair.reduced_crc;

	//Get the element on the bucket location
	tmp = &internal_hash[bucket];

	/* CHeck if the bucket is initialized. If it is that means this is the first
	 * element with that hash. Add the element and return that it is unique
	 */

	if (tmp->hash == 0 && tmp->pba == 0 && tmp->next == NULL){
		tmp->hash = pair.crc;
		tmp->pba = pba;
		tmp->next = NULL;

		id->unique = true;
		id->pba = pba;
	} else {
		/* This means that there is something at the bucket. Verify that the crc
		 * is the same. If same it is not unique. If it is same it is unique. Adjust
		 * output accordingly.
		 */
		if (tmp->hash == pair.crc){
			id->unique = false;
			id->pba = tmp->pba;
		} else {
			//This is the case when the actual crc doesnt match
			node = (kswish_internal_hash*)malloc(sizeof(kswish_internal_hash));
			if (node == NULL){
				rc = ENOENT;
				goto exit;
			}

			node->hash = pair.crc;
			node->pba = pba;
			node->next = NULL;
			tmp->next = node;

			id->unique = true;
			id->pba = pba;
		}
	}

exit:
	return rc;
}

/***********************************************************
 * Function: kswish_hash_clear
 * Objective: clear out the hash structure we have
 * Input: Nothing
 * Output: Nothing
 *
 ***********************************************************/
int kswish_hash_clear()
{
	int i = 0;
	int rc = EOK;
        kswish_internal_hash *tmp = NULL;
	kswish_internal_hash *prev_item = NULL, *next_item = NULL;

	 for (i = 0; i < MAX_HASH_BUCKET;  i++) {
                tmp = &internal_hash[i];
                tmp->hash = 0;
                tmp->pba = 0;

		if (tmp->next != NULL) {
			//This means we have a chain of nodes ahead
			prev_item = tmp->next;
			next_item = prev_item->next;
			while(prev_item !=NULL) {
				free (prev_item);
				prev_item = next_item;
				if (next_item != NULL)
					next_item = next_item->next;
			}
			tmp->next = NULL;
		}
        }

	 return rc;
}
