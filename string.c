
#include "types.h"
#include "x86.h"

void *memset(void *addr, int c, size_t n){

	c &= 0xff;

	if ((uintptr_t)addr % sizeof(uint64_t) == 0 ||
		 n % sizeof(uint64_t) == 0){
		uint64_t fill = c;
		fill = (fill << 56) | (fill << 48) | (fill << 40)
			 | (fill << 32) | (fill << 24) | (fill << 16)
			 | (fill << 8) |   fill;
		stosq(addr, fill, n/sizeof(uint64_t));	 
	} else if ((uintptr_t)addr % sizeof(uint32_t) == 0 ||
			 n % sizeof(uint32_t) == 0){
		uint32_t fill = c;
		fill = (fill << 24) | (fill << 16) | (fill << 8) | fill;
		stosl(addr, fill, n/sizeof(uint32_t));
	} else {
		uint8_t fill = c;
		stosb(addr, fill, n);
	}
	return addr;
}

void *memmove(void *dst, const void *src, size_t n){

	char *d;
	const char *s;

	d = dst;
	s = src;
	if (s < d && s + n > d){
		d += n;
		s += n;
		while (n-- > 0)
			*--d = *--s;
	} else
		while (n-- > 0)
			*d++ = *s++;

	return dst;
}

int memcmp(const void *v1, const void *v2, size_t n){

	const char *s1, *s2;

	s1 = v1;
	s2 = v2;
	while (n-- > 0){
		if (*s1 != *s2)
			return *s1 - *s2;
		s1++, s2++;	
	}

	return 0;
}

int strncmp(const char *s1, const char *s2, size_t n){

	while (n > 0 && *s1 && *s1 == *s2)
		n--,s1++, s2++; 	

	if (n == 0) return 0;

	return (uint8_t)*s1 - (uint8_t)*s2;
}

size_t strlen(const char *s){

	size_t n = 0;
	for (n = 0; s[n]; n++);

	return n;
}

char *strncpy(char *dst, const char *src, size_t n){

	char *d = dst;
	const char *s = src;
	while (n-- > 0 && *s)
		*d++ = *s++;
	
	while (n-- > 0)
		*d++ = 0;

	return dst;
}

char *safestrcpy(char *dst, const char *src, size_t n){

	char *d = dst;
	const char *s = src;

	if (n <= 0)
		return dst;

	while (--n > 0 && *s)
		*d++ = *s++;

	*d = 0;
		
	return dst;
}
