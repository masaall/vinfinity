
/*OUTPUT_FORMAT("binary")
*/

/*
OUTPUT_FORMAT("elf64-x86-64", "elf64-x86-64", "elf64-x86-64")
*/
OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
ENTRY(start)

SECTIONS 
{
	. = 0x7e00;

	.text : {
		*(.text)
	}

	.data : {
		*(.data)
	}
	
}
