
.code16
.globl start

start:

	cli

	xor     %ax,%ax
	mov     %ax,%ds
	mov 	%ax,%es
	mov 	%ax,%ss

	mov 	%dl, boot_disk

	mov 	$0x7c00,%ax
	mov 	%ax,%sp

	mov 	$0x7e00,%bx
	mov 	$34,%al		#; read 34 sectors
	mov 	$0x00,%ch	#; from cylinder 0
	mov 	$0x02,%cl 	#; from sector 2 (counting from 1)
	mov 	$0x00,%dh	#; from head 0
	call	disk_read

	mov 	$0x7e00,%ax
	jmp     *%ax

spin:
	jmp     spin	

print_16:
	lodsb	
	or      %al,%al
	jz      .done
	mov     $0x0e,%ah
	int     $0x10
	jmp     print_16

.done:
	ret		

disk_read:
	mov     $0x02,%ah
	mov     boot_disk,%dl
	int     $0x13

	jc      disk_error

	mov     $disk_success_string,%si
	call    print_16

	ret

disk_error:
	mov     $disk_error_string,%si
	call    print_16

	hlt	

disk_error_string:
	.asciz "disk read error!"	

disk_success_string:
	.asciz "disk success!"	
	
boot_disk:
	.byte 0
	
.fill	510-(.-start), 1, 0
.word	0xaa55

#; int 0x13: disk I/O
#; ah = 0x02 - read sector from drive
#; al = sector to read count
#; ch = cylinder number
#; cl = sector number
#; dh = head number
#; dl = drive number
#; es:bx = buffer address
