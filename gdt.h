
typedef struct {
	uint16_t limit;
	uint16_t base_low;
	uint8_t base_middle;
	uint8_t access;
	uint8_t gran;
	uint8_t base_high;
} __attribute__((packed)) gdt;

typedef struct {
	uint32_t pad0;
	uintptr_t rsp[3];
	uintptr_t pad1;
	uintptr_t ist[7];
	uintptr_t pad2;
	uint16_t pad3;
	uint16_t iomb;	
} __attribute__((packed)) tss;

typedef struct {
	uint32_t base_highest;
	uint32_t pad;	
} __attribute__((packed)) tss_ext;

typedef struct {
	uint16_t size;
	uintptr_t base;	
} __attribute__((packed)) pointer;

struct gdtAll {
	gdt entries[7];
	tss_ext tss_ext;	
	tss tss;
	pointer pointer;
};
