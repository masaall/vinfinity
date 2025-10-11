
#define T_IRQ0 32

#define IRQ_TIMER	0
#define IRQ_KBD		1
#define IRQ_COM1	4
#define IRQ_IDE	   14

struct idt {
	uint16_t off_low;
	uint16_t sel;
	uint8_t zero;
	uint8_t type;
	uint16_t off_mid;
	uint32_t off_high;
	uint32_t pad;
} __attribute__((packed));

struct regs {
	uintptr_t r15, r14, r13, r12, r11, r10, r9, r8;
	uintptr_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
	uintptr_t no, error;
	uintptr_t rip, cs, rflags, rsp, ss;	
};
