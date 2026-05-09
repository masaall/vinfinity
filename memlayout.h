
#define EXTMEM 0x100000
#define PHYSTOP 0xe000000
#define DEVSPACE 0xfe000000

#define KERNBASE 0xffffffff80000000
#define KERNDEV (0xffffffff00000000+DEVSPACE)

#define P2V(x) (void*)((uint8_t*)(x) + KERNBASE)
#define V2P(x) ((uintptr_t)(x) - KERNBASE)

#define P2V_DEV(x) (void*)((uint8_t*)(x) + 0xffffffff00000000)

#define V2P_WO(x) ((x) - KERNBASE)
