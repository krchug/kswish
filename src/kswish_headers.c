/**********************************************************
 * Filename: kswish_headers.c
 * Objective: Implement constructs for the various headers
 * Author: KC
 * Date: 31-12-2019
 * **********************************************************/

#include "kswish_headers.h"

/************************************************************
 * Function: init_supreme_header
 * Objective: Intialise supreme header
 * Input: Header pointer
 * Output: Initialized hheader
 ************************************************************/
static void init_supreme_header(kswish_supreme_header_st *hdr)
{
	hdr->sup_major_number = SUP_MAJ_NO;
	hdr->sup_minor_number = SUP_MINOR_NO;
	strcpy(hdr->sup_id_str, SUP_STR);
	hdr->header = KSWISH_NA;
}

/******************************************************
 * Function: kswish_send_metadata
 * Objective: Send the master record to the target
 * the order is spreme header->metadata header->mr
 * Input: node of mr, socket fd
 * Output: EOK on success
 ******************************************************/
int kswish_send_metadata(kswish_q_s *node, int sockfd)
{
	int rc = EOK;
	char *header_data;
	kswish_supreme_header_st *sup = NULL;
	kswish_metadata_header_st *meta = NULL;

	//allocate memory for header
	header_data = (char*)malloc(BLK_SIZE);
	if (header_data == NULL){
		rc = ENOENT;
		syslog(KSWISH_LOG_ERROR, "NET_PROD_q: Unable to allocate memory to header. Exiting");
                goto exit;
	}
	//Zero the block out
	memset( header_data, 0, BLK_SIZE);

	//Add the supreme header
	sup = (kswish_supreme_header_st*)header_data;
	init_supreme_header(sup);

	sup->header = KSWISH_META;

	//assign the meta
	meta = (kswish_metadata_header_st*) (header_data+sizeof(kswish_supreme_header_st));

	//init meta
	meta->major_number = META_MAJ_NO;
	meta->minor_number = META_MIN_NO;
	meta->blocks = 1;

	//Send it on the socket
	write(sockfd, header_data, BLK_SIZE);
	
	//Send our master record
	write(sockfd, node->data, BLK_SIZE);

	free(header_data);
exit:
	return rc;
}
int kswish_send_data(kswish_q_s *node, int sockfd, int blocks)
{
	int rc = EOK;
	char *header_data;
	kswish_supreme_header_st *sup = NULL;
	kswish_data_header_st *data_header = NULL;
	kswish_q_s *tmp = NULL;
	syslog(KSWISH_LOG_ERROR, " NET_PROD_Q:Total blocks to send %d", blocks);
	//allocate memory for header
	header_data = (char*)malloc(BLK_SIZE);
	if (header_data == NULL){
		rc = ENOENT;
		syslog(KSWISH_LOG_ERROR, "NET_PROD_q: Unable to allocate memory to header. Exiting");
		goto exit;
	}
	//Zero the block out
	memset( header_data, 0, BLK_SIZE);

	//Add the supreme header
	sup = (kswish_supreme_header_st*)header_data;
	init_supreme_header(sup);

	sup->header = KSWISH_DATA;

	//assign the data_header
	data_header = (kswish_data_header_st*) (header_data+sizeof(kswish_supreme_header_st));

	//init meta
	data_header->major_number = META_MAJ_NO;
	data_header->minor_number = META_MIN_NO;
	data_header->blocks_toexpect = blocks;

	//Send it on the socket
	syslog(KSWISH_LOG_ERROR, " NET_PROD_Q:Sending header data now");
	write(sockfd, header_data, BLK_SIZE);

	syslog(KSWISH_LOG_ERROR, " NET_PROD_Q:Sending blocks now");
	//Once done send our data blocks on the socket
	while (node != NULL){
		//Cache the node
		tmp = node;
		//Write on socket
		write(sockfd, tmp->data, BLK_SIZE);
		//Move tmp ahead
		node = node->next;
		//free the node once written
		free(tmp);
		//decrease count
		blocks -= 1;
	}

	if (blocks !=0){
		//This means something wrong has happened
		syslog(KSWISH_LOG_ERROR, "NET_PROD_Q: somethings wrong has happened. count %d", blocks);
		rc = ENOENT;
		goto exit;
	}

exit:
	return rc;
}

static int validate_supreme_header(kswish_supreme_header_st *sup)
{
	if (sup->sup_major_number != SUP_MAJ_NO)	return EINVAL;
	if (sup->sup_minor_number != SUP_MINOR_NO)	return EINVAL;
	if (strcmp(sup->sup_id_str, SUP_STR) != 0)	return EINVAL;

	return EOK;
}

static int validate_meta_header(kswish_metadata_header_st *meta)
{
	if (meta->major_number != META_MAJ_NO)		return EINVAL;
	if (meta->minor_number != META_MIN_NO)		return EINVAL;

	return EOK;
}
int kswish_recieve_metadata(int sockfd)
{
	int rc = EOK, count=0;
	char buff[4096];
        kswish_supreme_header_st *sup = NULL;
        kswish_metadata_header_st *meta_header = NULL;
	kswish_q_s *node = NULL;

	//Get the first block
	read(sockfd, buff, sizeof(buff));
	//This block will contain the supreme header and metadata header
	sup = (kswish_supreme_header_st*)buff;
	syslog(KSWISH_LOG_ERROR, "NET_CON_Q:Recieved supereme header");
	//Validate supremem header
	rc = validate_supreme_header(sup);
	if (rc != EOK) {
		syslog(KSWISH_LOG_ERROR, "NET_con_Q: Unable to validate supreme header");
                rc = ENOENT;
                goto exit;
	}
	syslog(KSWISH_LOG_ERROR, "NET_CON_Q:Supreme header validation passed");
	//Just a check if the header is meta which is next
	if (sup->header != KSWISH_META) {
		syslog(KSWISH_LOG_ERROR, "NET_con_Q: Unable to validate header type meta");
                rc = ENOENT;
                goto exit;
        }
	syslog(KSWISH_LOG_ERROR, "NET_CON_Q:Verified the next header is meta header");
	//Get the meta header from the data
	meta_header = (kswish_metadata_header_st*)(buff+sizeof(kswish_supreme_header_st));

	//Validate the meta header
	rc = validate_meta_header(meta_header);
	if (rc != EOK) {
                syslog(KSWISH_LOG_ERROR, "NET_con_Q: Unable to validate meta header");
                rc = ENOENT;
                goto exit;
        }
	syslog(KSWISH_LOG_ERROR, "NET_CON_Q:Validate meta header passed");
	//Take out the count
	count = meta_header->blocks;
	syslog(KSWISH_LOG_ERROR, "NET_CON_Q:Total blocks to read from network %d", count);
	while(count != 0) {
		//Count will be 1 only since we are sending only a single mr 
		//Read the mr
		syslog(KSWISH_LOG_ERROR, "NET_CON_Q:Reading master record");
		read(sockfd, buff, sizeof(buff));
		//create a node 
		rc = kswish_q_node_create(&node);
                if (rc != EOK) {
                        syslog(KSWISH_LOG_ERROR, "NET_CON_Q:Unable to create node for q. Exiting");
                        goto exit;
                }
		syslog(KSWISH_LOG_ERROR, "NET_CON_Q:Adding the block to the net conn q");
		//Add it to the net con q
		memcpy(node->data, buff, BLK_SIZE);
		//The first will always be the mr
                rc = kswish_q_add(node, NET_CON_Q);
                if (rc != EOK) {
                        syslog(KSWISH_LOG_ERROR, "NET_CON_Q:Unable to create node for q. Exiting");
                        goto exit;
                }
		syslog(KSWISH_LOG_ERROR, "NET_CON_Q:Success");
		count --;
		syslog(KSWISH_LOG_ERROR, "NET_CON_Q: count is %d", count);
	}
exit:
	return rc;
}
static int validate_data_header(kswish_data_header_st *meta)
{
	if (meta->major_number != META_MAJ_NO)		return EINVAL;
	if (meta->minor_number != META_MIN_NO)		return EINVAL;

	return EOK;
}

int kswish_recieve_data(int sockfd)
{
	int rc = EOK, count=0;
	char buff[4096];
        kswish_supreme_header_st *sup = NULL;
        kswish_data_header_st *data_header = NULL;
	kswish_q_s *node = NULL;

	//Get the first block
	syslog(KSWISH_LOG_ERROR, "NET_CON_Q: Now recieving data blocks");
	read(sockfd, buff, sizeof(buff));
	//This block will contain the supreme header and data header
	sup = (kswish_supreme_header_st*)buff;

	//Validate supremem header
	rc = validate_supreme_header(sup);
	if (rc != EOK) {
		syslog(KSWISH_LOG_ERROR, "NET_con_Q: Unable to validate supreme header");
                rc = ENOENT;
                goto exit;
	}
	syslog(KSWISH_LOG_ERROR, "NET_CON_Q:Validate supreme header passed");
	//Just a check if the header is data which is next
	if (sup->header != KSWISH_DATA) {
		syslog(KSWISH_LOG_ERROR, "NET_con_Q: Unable to validate header type data");
                rc = ENOENT;
                goto exit;
        }
	syslog(KSWISH_LOG_ERROR, "NET_CON_Q:Validate data header passed");
	//Get the data header from the data
	data_header = (kswish_data_header_st*)(buff+sizeof(kswish_supreme_header_st));

	//Validate the data header
	rc = validate_data_header(data_header);
	if (rc != EOK) {
                syslog(KSWISH_LOG_ERROR, "NET_con_Q: Unable to validate meta header");
                rc = ENOENT;
                goto exit;
        }
	syslog(KSWISH_LOG_ERROR, "NET_CON_Q:calidation passed");
	//Take out the count
	count = data_header->blocks_toexpect;
	syslog(KSWISH_LOG_ERROR, "NET_CON_Q:Total data blocks to expect %d", count);

	while(count != 0) {
		//Count will be multiple since we are sending multiple blocks 
		syslog(KSWISH_LOG_ERROR, "NET_CON_Q:Getting data block now");
		read(sockfd, buff, sizeof(buff));
		//create a node 
		rc = kswish_q_node_create(&node);
                if (rc != EOK) {
                        syslog(KSWISH_LOG_ERROR, "NET_CON_Q:Unable to create node for q. Exiting");
                        goto exit;
                }
		syslog(KSWISH_LOG_ERROR, "NET_CON_Q:Adding to net con q");
		//Add it to the net con q
                memcpy(node->data, buff, BLK_SIZE);
                rc = kswish_q_add(node, NET_CON_Q);
                if (rc != EOK) {
                        syslog(KSWISH_LOG_ERROR, "NET_CON_Q:Unable to create node for q. Exiting");
                        goto exit;
                }
		count --;
		syslog(KSWISH_LOG_ERROR, "NET_CON_Q:Success. Count is now %d", count);
	}
exit:
	return rc;

}
