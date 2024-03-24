/**********************************************************
 * Filename: kswish_headers.h
 * Objective: Implement constructs for the various headers
 * Author: KC
 * Date: 31-12-2019
 * **********************************************************/

#ifndef KSWISH_HDRS
#define KSWISH_HDRS

#include "kswish_types.h"
#include "kswish_log.h"
#include "kswish_queue.h"

#define SUP_MAJ_NO	255					//Supreme header major number
#define SUP_MINOR_NO	999					//Supreme header minor number
#define SUP_STR		"SUPREMEHEADERIDENTIFICATION"		//Suprement header identification
#define SUP_STR_LEN	32					//Supremen header id len

#define META_MAJ_NO	999					//Metadata header major
#define META_MIN_NO	255					//Metadata header minor

typedef enum kswish_header_type_e {
	KSWISH_META	= 1,
	KSWISH_DATA	= 2,
	KSWISH_NA	= 3,
} kswish_header_type_et;

typedef struct kswish_supreme_header_s {
	int 			sup_major_number;			//Major number
	int 			sup_minor_number;			//Minor number
	char 			sup_id_str[SUP_STR_LEN];		//id string
	kswish_header_type_et	header;					//next header to expect
} kswish_supreme_header_st;

typedef struct kswish_metadata_header_s {
	int			major_number;				//major number
	int 			minor_number;				//minor number
	int			blocks;					//how many meta blocks
} kswish_metadata_header_st;

typedef struct kswish_data_header_s {
	int			major_number;				//major number
	int 			minor_number;				//minor number
	int			blocks_toexpect;			//how many blocks to expect
} kswish_data_header_st;

int kswish_send_metadata(kswish_q_s *node, int sockfd);
int kswish_send_data(kswish_q_s *node, int sockfd, int blocks);

int kswish_recieve_metadata(int sockfd);
int kswish_recieve_data(int sockfd);
#endif
