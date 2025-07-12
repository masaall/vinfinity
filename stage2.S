
.code16
.globl stage2
stage2:

	lgdt	gdtdesc
	movl	%cr0,%eax
	orl     $0x1,%eax
	movl	%eax,%cr0

	ljmp	$0x8,$start32

.code32
start32:

	movw	$0x10,%ax
	movw	%ax,%ds
	movw	%ax,%es
	movw	%ax,%ss

	movl	$test,%esi
	call	print_32

	movl	$0x1,%eax
	cpuid
	testl	$(1 << 6),%edx
	jnz     pae_supported

	movl	$no_pae_string,%esi
	call	print_32

	hlt

pae_supported:	

	movl	$0x80000001,%eax
	cpuid
	testl	$(1 << 29),%edx
	jnz 	long_mode_supported

	movl	$no_long_mode_string,%esi
	call	print_32

	hlt

long_mode_supported:

	movl	$boot_pml4t,%edi
	movl	$1024,%ecx
	xorl	%eax,%eax
	rep     stosl 	

	movl	$boot_pml4t,%edi
	movl	$boot_pdpt, 0(%edi)
	orl     $0x3, 0(%edi)

	movl	$boot_pdpt,%edi
	movl	$1024,%ecx
	xorl	%eax,%eax
	rep 	stosl

	movl	$boot_pdpt,%edi
	movl	$(boot_pd + 3), 0(%edi)

	movl	$boot_pd,%edi
	movl	$1024,%ecx
	xorl	%eax,%eax
	rep     stosl

	movl	$boot_pd,%edi
	movl	$0x00000083, 0(%edi)

	movl	$boot_pml4t,%eax
	movl	%eax,%cr3	

	movl	%cr4,%eax
	orl     $0x30,%eax
	movl	%eax,%cr4

	mov 	$0xc0000080,%ecx
	rdmsr
	or      $(1 << 8),%eax
	wrmsr

	movl	%cr0,%eax
	orl     $0x80000000,%eax
	movl	%eax,%cr0

	lgdt	gdtdesc64
	
	ljmp	$0x08, $start64	

.code64
start64:

	mov 	$0x10,%ax
	mov 	%ax,%ds
	mov 	%ax,%es
	mov 	%ax,%ss

	mov 	$test64,%rsi
	call	print_64

	call	bootmain
	
spin:
	jmp     spin	

.code32
print_32:
	pushl	%ebp
	movl	%esp,%ebp

	mov 	$0xb8000,%ecx

.print_loop:
	lodsb	
	test 	%al,%al
	jz      .print_end

	mov 	$0x1a,%ah

	movw	%ax,(%ecx)
	add     $2,%ecx

	jmp     .print_loop	

.print_end:	
	movl	%ebp,%esp
	popl	%ebp
	ret

.code64
print_64:

	mov 	$0xb8000,%rcx

.print_loop64:
	lodsb	
	test 	%al,%al
	jz      .print_end64

	mov 	$0x1a,%ah

	movw	%ax,(%rcx)
	add     $2,%rcx

	jmp     .print_loop64	

.print_end64:	
	ret	

.align 4
gdt:
	.quad 0

	.word 0xffff
	.word 0
	.byte 0
	.byte 0x9a
	.byte 0xcf
	.byte 0

	.word 0xffff
	.word 0
	.byte 0
	.byte 0x92
	.byte 0xcf
	.byte 0

gdtdesc:
	.word	(gdtdesc - gdt - 1)
	.long	gdt	

.align 8
gdt64:

	.quad 0

	.word 0xffff
	.word 0
	.byte 0
	.byte (1 << 7) | (1 << 4) | (1 << 3) | (1 << 1)
	.byte (1 << 7) | (1 << 5) | 0xf
	.byte 0

	.word 0xffff
	.word 0
	.byte 0
	.byte (1 << 7) | (1 << 4) | (1 << 1)
	.byte (1 << 7) | (1 << 6) | 0xf
	.byte 0

gdtdesc64:
	.word	(gdtdesc64 - gdt64 - 1)
	.quad	gdt64
	
test:
	.asciz 	"32 bit"

test64:
	.asciz  "64 bit"	

msg:
	.asciz "hello 64 bit"	

no_pae_string:
	.asciz "computer does not support PAE!."

no_long_mode_string:
	.asciz "computer does not support long mode!."
	
.align 4096
.globl boot_pml4t
boot_pml4t:
	.skip 4096
.globl boot_pdpt
boot_pdpt:
	.skip 4096
.globl boot_pd0
boot_pd:
	.skip 4096

