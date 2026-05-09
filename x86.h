
static inline uint8_t inb(uint16_t port){
	uint8_t data;
	asm volatile("inb %1,%0" : "=a" (data) : "d" (port));
	return data;
}

static inline void outb(uint16_t port, uint8_t data){
	asm volatile("outb %0,%1" :: "a" (data), "d" (port));
}

static inline void insl(uint16_t port, void *addr, uint64_t cnt){
	asm volatile("rep insl" :: "d" (port), "D" (addr), "c" (cnt));
}

static inline void stosb(void *addr, uint8_t data, uint64_t cnt){
	asm volatile("rep stosb" :: "D" (addr), "a" (data), "c" (cnt));
}

static inline void stosw(void *addr, uint16_t data, uint64_t cnt){
	asm volatile("rep stosw" :: "D" (addr), "a" (data), "c" (cnt));
}

static inline void stosl(void *addr, uint32_t data, uint64_t cnt){
	asm volatile("rep stosl" :: "D" (addr), "a" (data), "c" (cnt));
}

static inline void stosq(void *addr, uint64_t data, uint64_t cnt){
	asm volatile("rep stosq" :: "D" (addr), "a" (data), "c" (cnt));
}

static inline void lcr3(uintptr_t addr){
	asm volatile("mov %0,%%cr3" :: "r" (addr));
}

static inline uint64_t rcr2(void){
	uint64_t val;
	asm volatile("mov %%cr2,%0" : "=r" (val));
	return val;
}

static inline uint64_t readrflags(void){
	uint64_t val;
	asm volatile("pushfq; pop %0" : "=r" (val));
	return val;
}

static inline void cli(void){
	asm volatile("cli");
}

static inline void sti(void){
	asm volatile("sti");
}

static inline uint64_t rdmsr(uint32_t msr){

	uint32_t low, high;

	asm volatile("rdmsr" : "=a" (low), "=d" (high) : "c" (msr));

	return (uint64_t)high << 32 | low;
}

static inline void wrmsr(uint32_t msr, uint64_t data){

	uint32_t low, high;

	asm volatile("rdmsr" : "=a" (low), "=d" (high) : "c" (msr));

	low |= data;
	high |= data >> 32;

	asm volatile("wrmsr" :: "a" (low), "d" (high), "c" (msr));
}
