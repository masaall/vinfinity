

static inline uint8_t inb(uint16_t port){
	uint8_t data;
	asm volatile("in %1,%0" : "=a" (data) : "d" (port));
	return data;	
}

static inline void outb(uint16_t port, uint8_t data){
	asm volatile("out %0,%1" : : "a" (data), "d" (port));
}

static inline void insl(int32_t port, void *addr, int32_t cnt){
	asm volatile("cld; rep insl" :
				 "=D" (addr), "=c" (cnt) :
				 "d" (port), "0" (addr), "1" (cnt) :
				 "memory", "cc");
}

static inline void stosb(void *addr, int8_t data, int64_t cnt){
	asm volatile("cld; rep stosb" :
				 "=D" (addr), "=c" (cnt) :
				 "0" (addr), "1" (cnt), "a" (data) :
				 "memory", "cc");
}

static inline void stosl(void *addr, int64_t data, int64_t cnt){
	asm volatile("cld; rep stosl" :
				 "=D" (addr), "=c" (cnt) :
				 "0" (addr), "1" (cnt), "a" (data) :
				 "memory", "cc");
}

static inline void stosq(void *addr, int64_t data, int64_t cnt){
	asm volatile("cld; rep stosq" :
				 "+D" (addr), "+c" (cnt) :
				 "a" (data) :
				 "memory", "cc");
}

static inline void lcr3(uint64_t val){
	asm volatile("movq %0,%%cr3" : : "r" (val));
}

struct segdesc;

static inline void lgdt(struct segdesc *p, int32_t size){

	struct {
		uint16_t limit;
		uint64_t base;
	} __attribute__((packed)) gdtr = {
		.limit = size - 1,
		.base = (uintptr_t)p
	};

	asm volatile("lgdt %0" : : "m" (gdtr));
}

static inline uint64_t readrflags(void){
	uint64_t rflags;
	asm volatile("pushfq; popq %0" : "=r" (rflags));
	return rflags;
}

static inline void cli(void){
	asm volatile("cli");
}

static inline void sti(void){
	asm volatile("sti");
}

static inline uint64_t xchg(volatile uintptr_t *addr, uint64_t newval){

	uint64_t result;

	asm volatile("lock; xchgq %0,%1" :
				 "+m" (*addr), "=a" (result) :
				 "1" (newval) :
				 "cc", "memory");

	return result;
}
