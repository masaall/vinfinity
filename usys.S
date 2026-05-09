

.globl write
write:
	mov $1,%rax
	syscall
	ret

.globl fork
fork:
	mov $2,%rax
	syscall
	ret

.globl wait
wait:
	mov $3,%rax
	syscall
	ret

.globl exit
exit:
	mov $4,%rax
	syscall
	ret

.globl getpid
getpid:
	mov $5,%rax
	syscall
	ret

.globl exec
exec:
	mov $6,%rax
	syscall
	ret
