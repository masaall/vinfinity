
OUTPUT_FORMAT("elf64-x86-64", "elf64-x86-64", "elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)

SECTIONS 
{
	. = 0xffffffff80100000; 

	PROVIDE(text = .);

	.text : AT(0x100000){
		*(.text .stub .text.* .gnu.linkonce.t.*)
	}

	.rodata : {
		*(.rodata .rodata.* .gnu.linkonce.r.*)
	}

	.stab : {
		PROVIDE(__STAB_BEGIN__ = .);
		*(.stab);
		PROVIDE(__STAB_END__ = .);
	}

	.stabstr : {
		PROVIDE(__STABSTR_BEGIN__ = .);
		*(.stabstr);
		PROVIDE(__STABSTR_END__ = .);
	}

	. = ALIGN(0x1000);

	PROVIDE(data = .);

	.data : {
		*(.data .data.*)
	}

	PROVIDE(bss = .);

	.bss : {
		*(.bss)
	}

	PROVIDE(end = .);
	
	/DISCARD/ : {
		*(.eh_frame .note.GNU-stack)
	}
}
