

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

static inline void insq(int64_t port, void *addr, int64_t cnt){
	asm volatile("cld; rep insq" :
				 "=D" (addr), "=c" (cnt) :
				 "d" (port), "0" (addr), "1" (cnt) :
				 "memory", "cc");
}

static inline void outsl(int32_t port, const void *addr, int32_t cnt){
	asm volatile("cld; rep outsl" :
				 "=S" (addr), "=c" (cnt) :
				 "d" (port), "0" (addr), "1" (cnt) :
				 "cc");
}

static inline void stosb(void *addr, int8_t data, int64_t cnt){
	asm volatile("cld; rep stosb" :
				 "=D" (addr), "=c" (cnt) :
				 "0" (addr), "1" (cnt), "a" (data) :
				 "memory", "cc");
}

static inline void stosl(void *addr, int32_t data, int64_t cnt){
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
/*
static inline uint64_t rdmsr(uint32_t id){
	uint32_t low, high;

	asm volatile ("rdmsr" :
				  "=a" (low), "=d" (high) :
				  "c" (id));

	return ((uint64_t)high << 32) | low;			  
}

static inline void wrmsr(uint32_t id, uint64_t value){
	uint32_t low = (uint32_t)(value & 0xffffffff);
	uint32_t high = (uint32_t)(value >> 32);

	asm volatile("wrmsr" :
				 		 :
				 "c" (id), "a" (low), "d" (high));
}*/


static inline uint64_t rdmsr(uint64_t id){
	uint32_t low, high;

	asm volatile ("rdmsr" :
				  "=a" (low), "=d" (high) :
				  "c" (id));

	return ((uint64_t)high << 32) | low;			  
}

static inline void wrmsr(uint64_t id, uint64_t value){
	uint32_t low = (uint32_t)(value & 0xffffffff);
	uint32_t high = (uint32_t)(value >> 32);

	asm volatile("wrmsr" :
				 		 :
				 "c" (id), "a" (low), "d" (high));
}

static inline void lcr3(uintptr_t val){
	asm volatile("movq %0,%%cr3" : : "r" (val));
}

struct gatedesc;

static inline void lidt(struct gatedesc *p, int32_t size){

	struct {
		uint16_t limit;
		uint64_t base;	
	} __attribute__((packed)) ldtr = {
		.limit = size - 1,
		.base = (uintptr_t)p
	};

	asm volatile("lidt %0" : : "m" (ldtr));
}

static inline void ltr(uint16_t sel){
	asm volatile("ltr %0" : : "r" (sel));
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

struct trapframe {
	uintptr_t r15;
	uintptr_t r14;
	uintptr_t r13;
	uintptr_t r12;
	uintptr_t r11;
	uintptr_t r10;
	uintptr_t r9;
	uintptr_t r8;
	
	uintptr_t rbp;
	uintptr_t rdi;
	uintptr_t rsi;
	uintptr_t rdx;
	uintptr_t rcx;
	uintptr_t rbx;
	uintptr_t rax;

	uintptr_t trapno;
	uintptr_t error;

	uintptr_t rip;
	uintptr_t cs;
	uintptr_t rflags;
	uintptr_t rsp;
	uintptr_t ss;
} __attribute__((packed));

struct syscallframe {
	uintptr_t r15;
	uintptr_t r14;
	uintptr_t r13;
	uintptr_t r12;
	uintptr_t r11;
	uintptr_t r10;
	uintptr_t r9;
	uintptr_t r8;
	
	uintptr_t rbp;
	uintptr_t rdi;
	uintptr_t rsi;
	uintptr_t rdx;
	uintptr_t rcx;
	uintptr_t rbx;
	uintptr_t rax;

	uintptr_t rip;
	uintptr_t cs;
	uintptr_t rflags;
	uintptr_t rsp;
	uintptr_t ss;
} __attribute__((packed));
