/*********************************************************************
 * Filename: kswish_master_record.c
 * Objective: Implement functions for the master record
 * Author: KC
 * Date: 05-12-2019
 *
 *********************************************************************/


#include "kswish_master_record.h"


/************************************************************************
 * Function: kswish_create_master_record_block
 * Objective: Create the master record block, init it and return
 * Input: Nothing
 * Output: Initialized master record block and return value. EOK if passed.
 * also updated metadata of the block.
 *
 ************************************************************************/
//Create a master record block
int kswish_create_master_record_block(kswish_master_record **master_record, kswish_master_record_meta **metadata)
{
	int rc = EOK;
	kswish_master_record *tmp_mr = NULL;
	kswish_master_record_meta *tmp_mr_meta = NULL;

	//Allocate memory to master record and metadata
	tmp_mr = (kswish_master_record*)malloc(sizeof(kswish_master_record)* KSWISH_MR_MAX_CNT);
	if (tmp_mr == NULL){
		rc = ENOENT;
		goto exit;	
	}

	tmp_mr_meta = (kswish_master_record_meta*)malloc(sizeof(kswish_master_record_meta));
	if (tmp_mr_meta == NULL){
		rc = ENOENT;
		goto exit;
	}

	//intialize by zeroes
	memset(tmp_mr, 0, (sizeof(kswish_master_record)* KSWISH_MR_MAX_CNT));

	//Init the metadata with respective values
	tmp_mr_meta->free_entries = KSWISH_MR_MAX_CNT;
	tmp_mr_meta->first_free_entry = 0;

	//Put in the out parameters
	*master_record = tmp_mr;
	*metadata = tmp_mr_meta;

exit:
	return rc;
}

/************************************************************************
 * Function: kswish_insert_in_master_record
 * Objective: Inserts into the master record block the entry
 * Input: lba, blk_number, master_record, metadata
 * Output: Initialized entry with the value passed. 
 * EOK if passed. ENOENT if failure, ECANNOTBEUSED if the mr is full
 *
 ************************************************************************/
int kswish_insert_in_master_record(kswish_master_record *master_record, 
			kswish_master_record_meta *metadata, uint64_t lba, uint64_t blk_number)
{
	int rc = EOK;
	int free_entry = 0;
	int total_free_entries = 0;
	kswish_master_record *entry_to_write = NULL;

	//find the first free entry
	free_entry = metadata->first_free_entry;
	total_free_entries = metadata->free_entries;

	//Handle when this goes beyond
	if(free_entry >= KSWISH_MR_MAX_CNT && total_free_entries <= 0){
		//This means that the block is full
		rc = ECANNOTBEUSED;
		goto exit;
	}
	//Increment the free entry and reduce the free entries
	metadata->first_free_entry += 1;
	metadata->free_entries -= 1;

	entry_to_write = &master_record[free_entry];

	entry_to_write->blk_number = blk_number;
	entry_to_write->lba = lba;


exit:
	return rc;
}



