
static inline uint8_t inb(uint16_t port){
	uint8_t data;
	asm volatile("inb %1,%0" : "=a" (data) : "d" (port));
	return data;
}

static inline void outb(uint16_t port, uint8_t data){
	asm volatile("outb %0,%1" :: "a" (data), "d" (port));
}

static inline void insl(uint16_t port, void *addr, uint32_t cnt){
	asm volatile("rep insl"
				: "=D" (addr), "=c" (cnt)
				: "d" (port), "0" (addr), "1" (cnt)
				: "memory", "cc");
}

static inline void outsl(uint16_t port, void *addr, uint32_t cnt){
	asm volatile("rep outsl"
				: "=S" (addr), "=c" (cnt)
				: "d" (port), "0" (addr), "1" (cnt)
				: "cc");
}

static inline void stosb(void *addr, uint8_t data, uint32_t cnt){
	asm volatile("rep stosb"
				:
				: "D" (addr), "c" (cnt), "a" (data)
				: "memory", "cc");
}

static inline void stosl(void *addr, uint32_t data, uint32_t cnt){
	asm volatile("rep stosl"
				: 
				: "D" (addr), "c" (cnt), "a" (data)
				: "memory", "cc");
}

static inline void stosq(void *addr, uint64_t data, uint32_t cnt){
	asm volatile("rep stosq"
				: 
				: "D" (addr), "c" (cnt), "a" (data)
				: "memory", "cc");
}

static inline void lcr3(uintptr_t val){
	asm volatile("mov %0,%%cr3" :: "r" (val));
}

static inline uintptr_t rcr2(void){
	uintptr_t val;
	asm volatile("mov %%cr2,%0" : "=r" (val));
	return val;
}

static inline uintptr_t readrflags(void){
	uintptr_t flags;
	asm volatile("pushfq; popq %0" : "=r" (flags));
	return flags;
}

static inline void cli(void){
	asm volatile("cli");
}

static inline void sti(void){
	asm volatile("sti");
}

static inline void wrmsr(uint32_t msr, uint64_t data){

	uint32_t hi, lo;

	hi = data >> 32;
	lo = data;
	asm volatile("wrmsr"
				:: "c" (msr),
				 "d" (hi), "a" (lo));
}

static inline uint64_t rdmsr(uint32_t msr){

	uint32_t hi, lo;

	asm volatile("rdmsr"
				: "=d" (hi), "=a" (lo)
				: "c" (msr));

	return (uint64_t)(hi) << 32 | lo;
}

static inline uint64_t xchg(volatile uintptr_t *addr, uint64_t newval){

	uint64_t result;

	asm volatile("lock; xchgq %0,%1"
				: "+m" (*addr), "=a" (result) 
				: "1" (newval)
				: "cc");

	return result;
}
