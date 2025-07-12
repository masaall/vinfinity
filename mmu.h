
#define FL_IF		0x00000200	// interrupt enable

#define CR0_PE		0x00000001	// protection enable


#define SEG_KCODE	1	// kernel code
#define SEG_KDATA	2	// kernel data + stack
#define SEG_UCODE	3 	// user code
#define SEG_UDATA	4 	// user data + stack

#define NSEGS		6

#ifndef __ASSEMBLER__

struct segdesc {
	uint16_t lim_15_0;		// low bits of segment limit
	uint16_t base_15_0;		// low bits of segment base address
	uint8_t base_23_16;		// middle bits of segment base address
	uint8_t type : 4;		// segment type
	uint8_t s : 1;			// 0 = system, 1 = application
	uint8_t dpl : 2;		// descriptor privilege level
	uint8_t p : 1;			// present
	uint8_t lim_19_16 : 4;	// high bits of segment limit
	uint8_t avl : 1;		// available for software use
	uint8_t l : 1;			// 64-bit code segment
	uint8_t db : 1;			// default operation size
	uint8_t g : 1;			// granularity
	uint8_t base_31_24;		// high bits of segment address
};

#define SEG64(type, base, lim, dpl, long_mode) (struct segdesc)	\
{	((lim) >> 12) & 0xffff, (base) & 0xffff,					\
	((base) >> 16) & 0xff, (type), 1, (dpl), 1,					\
	((lim) >> 28) & 0xf, 0, (long_mode), !(long_mode),			\
	1, ((base) >> 24) & 0xff }

#endif

#define DPL_USER	0x3

#define STA_X	0x8
#define STA_W	0x2
#define STA_R	0x2

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
