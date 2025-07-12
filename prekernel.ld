

OUTPUT_FORMAT("elf64-x86-64", "elf64-x86-64", "elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(prekernel)

SECTIONS 
{
	. = 0x00100000;	

	.text : {
		*(.text .stub .text.* .gnu.linkonce.t.*)
	}

	.rodata : {
		*(.rodata .rodata.*)
	}

	.stab : {
		PROVIDE(__STAB_BEGIN__ = .);
		*(.stab);
		PROVIDE(__STAB_END__ = .);
	}

	. = ALIGN(0x1000);

	.data : {
		*(.data .data.*)
	}

	bss : {
		*(.bss .bss.*)
	}
}

