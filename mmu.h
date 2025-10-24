
#define FL_IF	0x00000200

#define DPL_USER 0x3

#define PGUP(a)	((a + (PGSIZE-1)) & ~(PGSIZE-1))
#define PGDN(a)	((a) & ~(PGSIZE-1))

#define PGSIZE  4096
#define HPGSIZE 0x200000
#define NENTRIES 512

#define PTE_P	0x001
#define PTE_W	0x002
#define PTE_U	0x004
#define PTE_PS	0x080

#define PMLX(a) ((a >> PMLTSHIFT) & 0x1ff)
#define PDPX(a) ((a >> PDPTSHIFT) & 0x1ff)
#define PDX(a)	((a >> PDSHIFT) & 0x1ff)
#define PTX(a)	((a >> PTSHIFT) & 0x1ff)

#define PGADDR(pmlt, pdpt, pgdir, pgtab, offset)		\
		(												\
		((uintptr_t)(pmlt)	<< PMLTSHIFT)	|			\
 		((uintptr_t)(pdpt)	<< PDPTSHIFT)	|			\
 		((uintptr_t)(pgdir)	<< PDSHIFT)		|			\
 		((uintptr_t)(pgtab)	<< PTSHIFT)		|			\
 		(offset)							|			\
 		0xffff000000000000)

#define PG_ADDR(a) 	((a) & ~0xfff)
#define PG_FLAG(a)	((a) &  0xfff)

#define PMLTSHIFT 39
#define PDPTSHIFT 30
#define PDSHIFT 21
#define PTSHIFT 12
