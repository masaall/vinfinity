

#define font_str_doubled(_s, _x, _y, _c) do {\
	const char *__s = (_s);\
	__typeof__(_x) __x = (_x);\
	__typeof__(_y) __y = (_y);\
	__typeof__(_c) __c = (_c);\
	font_str(__s, __x, __y, __c);\
} while (0);
