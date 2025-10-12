
#include "types.h"
#include "memlayout.h"
#include "irqs.h" 

#define IOAPIC 0xfec00000

#define REG_TABLE 0x10

struct ioapic {
	uint32_t reg;
	uint32_t pad[3];
	uint32_t data;	
};

struct ioapic *ioapic;

void ioapicwrite(uint32_t reg, uint32_t data){
	ioapic->reg = reg;
	ioapic->data = data;
}

void ioapicinit(void){
	ioapic = P2V_DEV(IOAPIC);
}

void ioapicenable(int irq, int cpunum){
	ioapicwrite(REG_TABLE+2*irq, T_IRQ0 + irq);
	ioapicwrite(REG_TABLE+2*irq+1, cpunum << 24);
}
