
#define T_DIVIDE		0		// divide error
#define T_DEBUG			1		// debug exception
#define T_NMI			2		// non-maskabl interrupt
#define T_BRKPT			3		// breakpoint
#define T_OFLOW			4		// overflow
#define T_BOUND			5		// bounds check
#define T_ILLOP			6		// illegal opcode
#define T_DEVICE		7		// device not available
#define T_DBLFLT		8		// double fault

#define T_TSS		   10		// invalid tas switch segment
#define T_SEGNP		   11		// segment not present
#define T_STACK		   12		// stack exception
#define T_GPFLT		   13		// general protection fault
#define T_PGFLT		   14		// page fault

#define T_FPERR		   16		// floating point error
#define T_ALIGN		   17		// alignment check
#define T_MCHK		   18		// machine check
#define T_SIMDERR      19		// SIMD floating point error

#define T_SYSCALL      127		// system call
#define T_DEFAULT 	  500		// cathall

#define T_IRQ0		   32		// 

#define IRQ_TIMER		0
#define IRQ_KBD			1
#define IRQ_COM1		4
#define IRQ_IDE		   14
#define IRQ_ERROR	   19
#define IRQ_SPURIOUS   31
