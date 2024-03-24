/**********************************************************
 * Filename: kswish_log.c
 * Objective: Implement constructs for logging
 * Author: KC
 * Date: 6-12-2019
 *
 **********************************************************/

#include "kswish_log.h"

FILE *kswish_log_fp;
static int kswish_id;
pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER; 

char* kswish_print_time()
{
	int size = 0;
	time_t t;
	char *buf;

	t=time(NULL); /* get current calendar time */

	char *timestr = asctime( localtime(&t) );
	timestr[strlen(timestr) - 1] = 0;  //Getting rid of \n

	size = strlen(timestr)+ 1 + 2; //Additional +2 for square braces
	buf = (char*)malloc(size);

	memset(buf, 0x0, size);
	snprintf(buf,size,"[%s]", timestr);

	return buf;
}
void kswishlog(int priority, char* filename, int line, char *fmt,...)
{
	va_list         list;
	char            *p, *r;
	int             e;

	pthread_mutex_lock(&lock);
	if (kswish_id > 0) {
		kswish_log_fp = fopen("/tmp/kswishlog.txt", "a");
	} else {
		kswish_log_fp = fopen ("/tmp/kswishlog.txt","w");
	}

	if (priority == KSWISH_LOG_INFO){
		fprintf(kswish_log_fp, "INFO: ");
	} else {
		fprintf(kswish_log_fp, "ERROR: ");
	}
	fprintf(kswish_log_fp,"%s ",kswish_print_time());
	fprintf(kswish_log_fp,"[%s][line: %d] ",filename,line);
	va_start( list, fmt );

	for ( p = fmt ; *p ; ++p )
	{
		if ( *p != '%' )//If simple string
		{
			fputc( *p,kswish_log_fp);
		}
		else
		{
			switch ( *++p )
			{
				/* string */
				case 's':
					{
						r = va_arg( list, char * );
						fprintf(kswish_log_fp,"%s", r);
						continue;
					}
					/* integer */
				case 'd':
					{
						e = va_arg( list, int );
						fprintf(kswish_log_fp,"%d", e);
						continue;
					}
				default:
					fputc( *p, kswish_log_fp);
			}
		}
	}
	va_end( list );
	kswish_id++;
	fputc( '\n', kswish_log_fp);
	fflush(kswish_log_fp);
	fclose(kswish_log_fp);
	pthread_mutex_unlock(&lock);
}
