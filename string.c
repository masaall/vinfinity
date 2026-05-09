
#include "types.h"
#include "defs.h"
#include "x86.h"

void *memset(void *addr, int c, size_t n){

	c &= 0xff;

	if (!((uintptr_t)addr & 7) && !(n & 7)){
		uint64_t fill = c;
		fill = fill << 56 | fill << 48 | fill << 40 | fill << 32
			 | fill << 24 | fill << 16 | fill << 8 | fill;
		stosq(addr, fill, n / 8);
	} else if (!((uintptr_t)addr & 3) && !(n & 3)){
		uint32_t fill = c;
		fill = fill << 24 | fill << 16 | fill << 8 | fill;
		stosl(addr, fill, n / 4);
	} else if (!((uintptr_t)addr & 1) && !(n & 1)){
		uint16_t fill = c;
		fill = fill << 8 | fill;
		stosw(addr, fill, n / 2);
	} else 
		stosb(addr, c, n);

	return addr;
}

int cnt;
/*
void *memmove(void *dst, const void *src, size_t n){

	uint8_t *d = dst;
	const uint8_t *s = src;

	if (d == s){
		return d;
	}

	if (s < d && s + n > d){

		if ((uintptr_t)d % 8 == (uintptr_t)s % 8){

			while ((uintptr_t)d+n % 8){

				if (n-- == 0)
					return dst;

				*(d+n) = *(s+n);
			}

			while (n >= 8){
				n -= 8;
				*(size_t*)(d+n) = *(size_t*)(s+n);				
			}
		}

		while (n-- > 0)
			*(d+n) = *(s+n);

	} else {

		if ((uintptr_t)d % 8 == (uintptr_t)s % 8){

			while ((uintptr_t)d % 8){

				if (n-- == 0)
					return dst;

				*d++ = *s++;
			}

			for (; n >= 8; n -= 8, d += 8, s += 8)
				*(size_t*)d = *(size_t*)s;
		}

		while (n-- > 0)
			*d++ = *s++;
	}

	return dst;
}
*/

void *memmove(void *dst, const void *src, size_t n){

	uint8_t *d = dst;
	const uint8_t *s = src;

	if (s < d && s + n > d){

		s += n;
		d += n;

		if ((uintptr_t)d % 8 == (uintptr_t)s % 8){
			while ((uintptr_t)d % 8){

				if (n-- == 0)
					return dst;

				*--d = *--s;
			}

			while (n >= 8){
				n -= 8, d -= 8, s -= 8;
				*(size_t*)d = *(size_t*)s;
			}
		}

		while (n-- > 0)
			*--d = *--s;
	} else {

		if ((uintptr_t)d % 8 == (uintptr_t)s % 8){
			while ((uintptr_t)d % 8){

				if (n-- == 0)
					return dst;

				*d++ = *s++;
			}

			while (n >= 8){
				*(size_t*)d = *(size_t*)s;
				n -= 8, d += 8, s += 8;
			}
		}
		while (n-- > 0)
			*d++ = *s++;
	}

	return dst;
}

int memcmp(const void *v1, const void *v2, size_t n){

	const uint8_t *s1 = v1;
	const uint8_t *s2 = v2;

	while (n-- > 0){
		if (*s1 != *s2)
			return *s1 - *s2;
		s1++, s2++;
	}

	return 0;
}

int strncmp(const char *s1, const char *s2, size_t n){

	while (n > 0 && *s1 && *s1 == *s2)
		n--, s1++, s2++;

	if (n == 0)
		return 0;

	return (uint8_t)*s1 - (uint8_t)*s2;
}
