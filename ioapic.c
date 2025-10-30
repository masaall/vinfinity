
#include "types.h"
#include "defs.h"
#include "memlayout.h"
#include "irqs.h" 

#define IOAPIC 0xfec00000

#define REG_ID 0x00
#define REG_VER 0x01
#define REG_TABLE 0x10

#define INT_DISABLED 0x00010000	// interrupt disabled

struct ioapic {
	uint32_t reg;
	uint32_t pad[3];
	uint32_t data;
};

volatile struct ioapic *ioapic;

uint32_t ioapicread(uint32_t reg){
	ioapic->reg = reg;
	return ioapic->data;
}

void ioapicwrite(uint32_t reg, uint32_t data){
	ioapic->reg = reg;
	ioapic->data = data;
}

void ioapicinit(void){

	int i, id, maxintr;

	ioapic = P2V_DEV(IOAPIC);
	maxintr = (ioapicread(REG_VER) >> 16) & 0xff;
	id = ioapicread(REG_ID) >> 24;
	if (id != ioapicid)
		cprintf("ioapicinit: id not equal \n");

	for (i = 0; i <= maxintr; i++){
		ioapicwrite(REG_TABLE+2*i, INT_DISABLED | (T_IRQ0 + i));
		ioapicwrite(REG_TABLE+2*i+1, 0);
	}	
}

void ioapicenable(int irq, int cpunum){
	ioapicwrite(REG_TABLE+2*irq, T_IRQ0 + irq);
	ioapicwrite(REG_TABLE+2*irq+1, cpunum << 24);
}
