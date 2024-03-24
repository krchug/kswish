/*********************************************************
 * File name: kswish_crc.h
 * Objective: Contains the headers for calculation of crc
 * Author: KC
 *
 *********************************************************/
#ifndef KSWISH_CRC
#define KSWISH_CRC

#include "kswish_types.h"
#include "kswish_log.h"

typedef struct crc_pair {
	uint32_t crc;			//We are using crc32. This will be actual crc
	uint32_t reduced_crc;		//This will be the reduced crc which is crc%256
} crc_pair;


int crc_return_from_base(const char *buf, size_t len, crc_pair *crc);

#endif
