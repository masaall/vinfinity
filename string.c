
#include "types.h"
#include "x86.h"

void *memset(void *dst, int32_t c, size_t n){

	c &= 0xff;

	if (((uintptr_t)dst % sizeof(uint64_t)) == 0 && (n % sizeof(uint64_t)) == 0){
		uint64_t pattern = (uint8_t)c;
		pattern = (pattern << 56) | (pattern << 48) | (pattern << 40)
				| (pattern << 32) | (pattern << 24) | (pattern << 16)
				| (pattern << 8) | pattern;
		stosq((uint64_t*)dst, pattern, n/sizeof(uint64_t));		
	}
	else if (((uintptr_t)dst % sizeof(uint32_t)) == 0 && (n % sizeof(uint32_t)) == 0){
		uint32_t pattern = (uint8_t)c;
		pattern = (pattern << 24) | (pattern << 16) | (pattern << 8)
				| pattern;
		stosl((uint32_t*)dst, pattern, n/sizeof(uint32_t));	
	}
	else {
		stosb(dst, c, n);
	}

	return dst;
}

uint32_t memcmp(const void *v1, const void *v2, size_t n){

	const uint8_t *s1, *s2;

	s1 = v1;
	s2 = v2;
	while (n-- > 0){
		if (*s1 != *s2)
			return *s1 - *s2;
		s1++, s2++;	
	}

	return 0;
}

void *memmove(void *dst, const void *src, size_t n){

	const char *s;
	char *d;

	s = src;
	d = dst;
	if (s < d && s + n > d){
		s += n;
		d += n;
		while (n-- > 0)
			*--d = *--s;
	} else
		while (n-- > 0)
			*d++ = *s++;

	return dst;
}
