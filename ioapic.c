
#include "types.h"
#include "defs.h"
#include "memlayout.h"
#include "traps.h"

#define IOAPIC 0xfec00000

#define REG_ID		0x00	// register index: ID
#define REG_VER		0x01	// register index: version
#define REG_TABLE	0x10	// redirection table base

#define INT_DISABLED	0x00010000	// interrupt disabled
#define INT_LEVEL		0x00008000	// level-triggered
#define INT_ACTIVELOW	0x00002000	// active low (vs high)
#define INT_LOGICAL		0x00000800	// destination is CPU id (vs APIC ID)

struct ioapic {
	uint32_t reg;	
	uint32_t pad[3];
	uint32_t data;
};

volatile struct ioapic *ioapic;

uint32_t ioapicread(int32_t reg){
	ioapic->reg = reg;
	return ioapic->data;
}

void ioapicwrite(int32_t reg, int32_t data){
	ioapic->reg = reg;
	ioapic->data = data;
}

void ioapicinit(void){

	uint32_t maxintr, id, i;

	ioapic = (volatile struct ioapic*)P2V_DEV(IOAPIC);
	maxintr = (ioapicread(REG_VER) >> 16) & 0xff;
	id = ioapicread(REG_VER) >> 24;
	if (id != ioapicid)
		cprintf("ioapicinit: id isn't equal to ioapicid; not a MP\n");

	for (i = 0; i <= maxintr; i++){
		ioapicwrite(REG_TABLE+2*i, INT_DISABLED | (T_IRQ0 + i));
		ioapicwrite(REG_TABLE+2*i+1, 0);
	}	
}

void ioapicenable(int32_t irq, int32_t cpunum){
	ioapicwrite(REG_TABLE+2*irq, T_IRQ0 + irq);
	ioapicwrite(REG_TABLE+2*irq+1, cpunum << 24);
}
