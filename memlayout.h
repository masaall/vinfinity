
#define EXTMEM	 0x100000 	
#define PHYSTOP  0xe000000		// top physical memory
#define DEVSPACE 0xfe000000		// device

#define KERNBASE 0xffffffff80000000	// first kernel virtual address
#define KERNLINK (KERNBASE+EXTMEM)
#define KERNDEV	 (0xffffffff00000000+DEVSPACE)

#define V2P(a) (((uintptr_t)(a)) - KERNBASE)
#define P2V(a) ((void*)(((char*)(a)) + KERNBASE))

#define P2V_DEV(a) ((void*)(((char*)(a)) + 0xffffffff00000000))

#define V2P_WO(x) ((x) - KERNBASE)
