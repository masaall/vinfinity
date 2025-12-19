
int fork(void);
int write(int, const void*, int);
int wait(void);
int getpid(void);
int exit(void) __attribute__((noreturn));
char *sbrk(int);
int open(const char*, int);
int mknod(const char*, short, short);
int dup(int);
int exec(char*, char**);

void putc(int, char);
void printf(const char*, ...);
void *malloc(uintptr_t);
void *memset(void*, int, size_t);
