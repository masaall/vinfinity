
struct segdesc {
	uint16_t limit;
	uint16_t base_low;
	uint8_t base_middle;
	uint8_t access;
	uint8_t gran;
	uint8_t base_high;
};

struct tss {
	uint32_t pad0;
	uint64_t rsp[3];
	uint64_t pad1;
	uint64_t ist[7];
	uint64_t pad2;
	uint16_t pad3;
	uint16_t iopb;
} __attribute__((packed));

struct tss_ext {
	uint32_t base_highest;
	uint32_t pad;
};

struct gdt_pointer {
	uint16_t size;
	uintptr_t base;
} __attribute__((packed));

struct gdtAll {
	struct segdesc entries[7];
	struct tss_ext tss_ext;
	struct tss tss;
	struct gdt_pointer pointer;
};
