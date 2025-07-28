
struct gdt {
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t base_middle;
	uint8_t access;
	uint8_t granularity;
	uint8_t base_high;	
} __attribute__((packed));

struct tss {
	uint32_t reserved0;
	uint64_t rsp[3];
	uint64_t reserved1;
	uint64_t isr[7];
	uint64_t reserved2;
	uint16_t reserved3;
	uint16_t iomb;	
} __attribute__((packed));

struct tss_ext {
	uint32_t base_highest;
	uint32_t reserved0;	
} __attribute__((packed));

struct pointer {
	uint16_t limit;
	uintptr_t base;	
} __attribute__((packed));

struct fullGDT {
	struct gdt entries[6];
	struct tss_ext tss_ext;
	struct pointer pointer;
	struct tss tss;	
} __attribute__((packed)) __attribute__((aligned(0x10)));

struct fullGDT gdt __attribute__ ((used)) = {
	{
		{0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00},
		{0xffff, 0x0000, 0x00, 0x9a, (1 << 5) | (1 << 7) | 0xf, 0x00},
		{0xffff, 0x0000, 0x00, 0x92, (1 << 5) | (1 << 7) | 0xf, 0x00},
		{0xffff, 0x0000, 0x00, 0xfa, (1 << 5) | (1 << 7) | 0xf, 0x00},
		{0xffff, 0x0000, 0x00, 0xf2, (1 << 5) | (1 << 7) | 0xf, 0x00},
		{0x0067, 0x0000, 0x00, 0xe9, 0x00, 0x00},
	},
	{0x00000000, 0x00000000},
	{0x0000, 0x0000000000000000},
	{0,{0,0,0},0,{0,0,0,0,0,0,0},0,0,0},
};
