/************************************************************************
 * Filename: kswish_crc.c
 * Objective: Crc related computations
 * Author: KC
 * Date: 04/12/2019
 *
 *************************************************************************/


#include "kswish_crc.h"



/*******************************************************
 * Function: crc_return_from_base
 * Objective: Claculate crc of the block and mod it with
 * 	      256 to get the value we need
 * Input: Const char*buf - Buffer
 * 	  Size_t len	 - Length of the buffer
 *
 * Output: crc_pair structure which contains actual
 *         crc value and the reduced value
 *         EOK if passed, ENOENT if failed
 *
 ******************************************************/
int crc_return_from_base(const char *buf, size_t len, crc_pair *crc)
{

	int rc = EOK;
	crc->crc =crc32(0, (const void*)buf, sizeof(buf));
        crc->reduced_crc = (crc->crc %256);

	syslog(KSWISH_LOG_ERROR, "Crc is %u and reduced crc is %u", crc->crc, crc->reduced_crc);
        printf("%u %u\n", crc->crc, crc->reduced_crc);

        return rc;

}

