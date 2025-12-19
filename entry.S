
#include "param.h"
#include "memlayout.h"

.globl _start
_start = V2P_WO(entry)
entry:

	mov $V2P_WO(entrypml5t),%rax
	mov %rax,%cr3

	mov $(stack + KSTACKSIZE),%rsp

	mov $main,%rax
	jmp *%rax

.comm stack, KSTACKSIZE
