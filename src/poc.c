#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>

#include "kswish_types.h"
#include "kswish_log.h"
#include "kswish_crc.h"
#include "kswish_internal_hash.h"
#include "kswish_master_record.h"

#define docker_file	"/home/kaushik/Desktop/inst360vids/LRV_20240109_134936_11_061.lrv"
#define disk1size 217896796
//Calculate the blocks in the disk
#define blocks (disk1size/4096)

int main ()
{

	FILE *fp = NULL;

	int j=0;
        long offset=0;
	int chunk_size = 256;
	char block[4096];
	int chunk;
	crc_pair crc;
	kswish_unique_return id;
	int rc = EOK, unique =0, dedupe = 0;
	int total_unique =0, total_dedupe=0;

	fp = fopen(docker_file, "r");
	if (fp==NULL){
		printf("Open failed for disk %s. Exiting", docker_file);

		return 0;
	}


	memset(block,0,4096);
	offset =0;
	chunk=0;
	for(j=1; j<=blocks; j++){
		//printf("\nDisk Seeking to %lu , %s", offset, docker_file);

		if (j%256 == 0){
			printf("\nIMP: Chunk %d unique %d dedupe %d\n", chunk, unique, dedupe);
			total_unique = total_unique + unique;
			total_dedupe = total_dedupe + dedupe;
			chunk++;
			kswish_hash_init();
			unique =0;
			dedupe =0;
		}

		if (fseek(fp, offset, SEEK_SET) != 0)
		{
			printf("Fseek failed");
			return 0;
		}

		fgets(block, 4096, fp);

		rc = crc_return_from_base(block, 4096, &crc);
                if (rc != EOK) {
                        rc = ENOENT;
                        goto exit;
                }

		rc = kswish_hash_add_check(crc, j%256, &id);
                if (rc != EOK) {
                        rc = ENOENT;
                        goto exit;
                }

		if (id.unique) {
			unique++;
		} else {
			dedupe++;
		}

		//printf("Block data %s", block);
		offset = offset +4096;
	}

exit:
	fclose(fp);
	printf("\nTotal chunks %d\n", chunk);
	printf("\nTotal unique blocks %d which amounts to %dMB of data \n", total_unique, ((total_unique *4096)/1024/1024));
	printf("\nTotal dedupe blocks %d which amounts to %dMB of data\n", total_dedupe, ((total_dedupe *4096)/1024/1024));


}
