/*********************************************************************
 * Filename: kswish_master_record.h
 * Objective: Implement headers for masterrecord
 * Author: KC
 * Date: 05-12-2019
 *
 *********************************************************************/
#ifndef KSWISH_MR
#define KSWISH_MR

#include "kswish_types.h"



typedef struct kswish_master_record {
	uint64_t blk_number;		//Block number of the block in the sequence
	uint64_t lba;			//Logical block number
} kswish_master_record;


typedef struct kswish_master_record_meta {
	uint64_t first_free_entry;	//First free entry in the current master block
	uint64_t free_entries;		//free entries left
} kswish_master_record_meta;

#define	KSWISH_MR_SIZE		16	//Size of master record entry is 16 bytes
#define KSWISH_MR_MAX_CNT	(BLK_SIZE/KSWISH_MR_SIZE)

//Create a master record block
int kswish_create_master_record_block(kswish_master_record **master_record, kswish_master_record_meta **metadata);

//Insert into master record
int kswish_insert_in_master_record(kswish_master_record *master_record, kswish_master_record_meta *metadata, uint64_t lba, uint64_t blk_number);

#endif
