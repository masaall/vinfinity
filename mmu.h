
#define FL_IF	0x00000200

#define DPL_USER 0x3

#define PGUP(a)	(((uintptr_t)(a) + 0xfff) & ~0xfff)
#define PGDN(a)	((uintptr_t)(a) & ~0xfff)

#define PGSIZE  0x1000
#define HPGSIZE 0x200000
#define HHPGSIZE 0x40000000
#define NENTRIES 512

#define PTE_P	0x001
#define PTE_W	0x002
#define PTE_U	0x004
#define PTE_PS	0x080

#define PML5X(a) (((uintptr_t)(a) >> PML5SHIFT) & 0x1ff)
#define PML4X(a) (((uintptr_t)(a) >> PML4SHIFT) & 0x1ff)
#define PDPX(a) (((uintptr_t)(a) >> PDPTSHIFT) & 0x1ff)
#define PDX(a)	(((uintptr_t)(a) >> PDSHIFT) & 0x1ff)
#define PTX(a)	(((uintptr_t)(a) >> PTSHIFT) & 0x1ff)

#define PGADDR(pml4t, pdpt, pgdir, pgtab, offset)	\
		(												\
		((uintptr_t)(pml4t)	<< PML4SHIFT)	|			\
 		((uintptr_t)(pdpt)	<< PDPTSHIFT)	|			\
 		((uintptr_t)(pgdir)	<< PDSHIFT)		|			\
 		((uintptr_t)(pgtab)	<< PTSHIFT)		|			\
 		(offset)							|			\
 		0xffff000000000000)

#define PG_ADDR(a) 	((uintptr_t)(a) & ~0xfff)
#define PG_FLAG(a)	((uintptr_t)(a) &  0xfff)

#define PML5SHIFT 48
#define PML4SHIFT 39
#define PDPTSHIFT 30
#define PDSHIFT 21
#define PTSHIFT 12
