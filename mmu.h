
#define FL_IF		0x00000200	// interrupt enable

#define CR0_PE		0x00000001	// protection enable


#define SEG_KCODE	1	// kernel code
#define SEG_KDATA	2	// kernel data + stack
#define SEG_UCODE	3 	// user code
#define SEG_UDATA	4 	// user data + stack
#define SEG_TSS 	5

#define NSEGS		6

#define DPL_USER	0x3

#define STA_X	0x8
#define STA_W	0x2
#define STA_R	0x2

#define STS_IG	0xe		// interrupt gate
#define STS_TG	0xf 	// trap gate

#define PML4X(va)	(((uintptr_t)(va) >> PML4SHIFT) & 0x1ff)
#define PDPTX(va)	(((uintptr_t)(va) >> PDPTSHIFT) & 0x1ff)
#define PDX(va)		(((uintptr_t)(va) >> PDSHIFT) & 0x1ff)
#define PTX(va)		(((uintptr_t)(va) >> PTSHIFT) & 0x1ff)

#define PGADDR(pml4, pdpt, pd, pt, offset)  			\
	((uintptr_t)(										\
		(((uintptr_t)pml4) << PML4SHIFT)	|			\
		(((uintptr_t)pdpt) << PDPTSHIFT) 	|			\
		(((uintptr_t)pd) << PDSHIFT) 		|			\
		(((uintptr_t)pt) << PTSHIFT) 		| (offset) 	\
	)	| 												\
	0xffff000000000000									\
	) 

#define PGSIZE		4096

#define PML4SHIFT 	39
#define PDPTSHIFT	30
#define PDSHIFT		21
#define PTSHIFT		12

#define PGROUNDUP(a)	(((a)+PGSIZE-1) & ~(PGSIZE-1))
#define PGROUNDDOWN(a)	(((a)) & ~(PGSIZE-1))

#define PTE_P		0x001
#define PTE_W		0x002
#define PTE_U		0x004

#define PTE_ADDR(pg)	((uintptr_t)(pg) & ~0xfff)

#ifndef __ASSEMBLER__

struct gatedesc {
	uint32_t off_15_0 : 16;
	uint32_t selector : 16;

	uint32_t zero : 8;
	uint32_t type : 4;
	uint32_t s : 1;
	uint32_t dpl : 2;
	uint32_t p : 1;

	uint32_t off_31_16 : 16;
	uint32_t off_63_32;
	uint32_t pad;	
} __attribute__((packed));

#define SETGATE(gate, istrap, sel, off, d)		\
{												\
	(gate).off_15_0 = (uintptr_t)(off) & 0xffff; \
	(gate).selector = (sel);					\
	(gate).zero = 0; 							\
	(gate).type = (istrap) ? STS_TG : STS_IG;	\
	(gate).s = 0;								\
	(gate).dpl = (d);							\
	(gate).p = 1;								\
	(gate).off_31_16 = (uintptr_t)(off >> 16) & 0xffff;\
	(gate).off_63_32 = (uintptr_t)(off >> 32) & 0xffffffff;\
	(gate).pad = 0;								\
}


#endif
