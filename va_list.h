
typedef __builtin_va_list va_list;

#define va_start(pa, last) __builtin_va_start(pa, last)
#define va_end(pa) __builtin_va_end(pa)
#define va_arg(pa, type) __builtin_va_arg(pa, type)
