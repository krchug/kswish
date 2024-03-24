#include "kswish_types.h"
#include "kswish_log.h"
#include "kswish_crc.h"

int main ()
{

	const char s[4096] = "Hello World";
	crc_pair new_pair;
	int rc = EOK;

	rc = crc_return_from_base(s, 4096, &new_pair);
	KSWISHLOG(KSWISH_LOG_INFO, "Hello from info");
	KSWISHLOG(KSWISH_LOG_ERROR, "Hello from error");

	printf("Actual crc %x reduced crc %u", new_pair.crc, new_pair.reduced_crc);


	return rc;
}
