
#include "param.h"
#include "memlayout.h"
#include "mmu.h"

.globl _start
_start = V2P_WO(entry)
entry:

	movq $V2P_WO(entrypml4t),%rax
	movq %rax,%cr3

	movq $(stack + KSTACKSIZE),%rsp

	movq $main,%rax
	jmp *%rax

.comm stack, KSTACKSIZE
