
#include "types.h"
#include "spinlock.h"

#define PIPESIZE 512

struct pipe {
	struct spinlock lock;	
	char data[PIPESIZE];
	uint64_t nread;			// number of bytes read
	uint64_t nwrite;		// number of bytes written
	int32_t readopen;		// read fd is still open
	int32_t writeopen;		// write fd is still open	
};
