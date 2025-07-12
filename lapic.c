
#include "types.h"

#define ID 			(0x0020/4)		// ID

volatile uint32_t *lapic;

int32_t lapicid(void){

	if (!lapic)
		return 0;

	return *(lapic + ID) >> 24;	
}

