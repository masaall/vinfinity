
#include "types.h"
#include "x86.h"

void *memset(void *addr, int c, size_t n){

	c &= 0xff;

	if ((uintptr_t)addr%sizeof(uint64_t)==0 || n%sizeof(uint64_t)==0){
		uint64_t fill = c;
		fill |= (fill << 56) | (fill << 48) | (fill << 40)
			 |  (fill << 32) | (fill << 24) | (fill << 16)
			 |  (fill << 8) | fill;
		stosq(addr, fill, n/sizeof(uint64_t));	 
	} 
	else if ((uintptr_t)addr%sizeof(uint32_t)==0 || n%sizeof(uint32_t)==0){
		uint32_t fill = c;
		fill |= (fill << 24) | (fill << 16) | (fill << 8) | fill;
		stosl(addr, fill, n/sizeof(uint32_t));
	} else {
		uint8_t fill = c;
		stosb(addr, fill, n);
	}
	return addr;
}
