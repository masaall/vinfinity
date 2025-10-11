
.code16
.globl start
start:
	cli

	mov $0x7e00,%bx
	mov $34,%al
	mov $0,%ch
	mov $2,%cl
	mov $0,%dh
	call read_disk

/*	mov $0x00,%ah
	mov $0x13,%al
	int $0x10
*/
	mov $0x7e00,%ax
	jmp *%ax

read_disk:
	mov $0x2,%ah
	int $0x13
	jc read_disk_error

	mov $read_disk_success_str,%si
	call print

	ret

read_disk_error:
	mov $read_disk_error_str,%si
	call print
	
	hlt

print:
	lodsb
	or %al,%al
	jz .end
	mov $0xe,%ah
	int $0x10
	jmp print
.end:
	ret

read_disk_error_str:
	.asciz "read disk error!"
read_disk_success_str:
	.asciz "read disk success!"	

.fill (510-(.-start)), 1, 0
.word 0xaa55	

#; ah 02h
#; al sector to read count
#; ch cylinder
#; cl sector
#; dh head
#; dl drive
#; es:bx buffer address pointer
