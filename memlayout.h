
#define PHYSTOP 0xe000000
#define DEVSPACE 0xfe000000

#define KERNBASE 0xffffffff80000000
#define KERNDEV	 (0xffffffff00000000+DEVSPACE)

#define P2V(a) (void*)((char*)(a) + KERNBASE)
#define V2P(a) ((uintptr_t)(a) - KERNBASE)

#define P2V_DEV(a)	(void*)((char*)(a) + 0xffffffff00000000)

#define V2P_WO(a) ((a) - KERNBASE)
