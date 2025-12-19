# VinfinityOS

## Overview
A 64-bit operating system built from scratch.

## Features

### **Core System**
- **Custom Bootloader** - Handles transition to 64-bit mode
- **5-Level Paging** - PML5 -> PML4 -> PDP -> PD -> PT (x86-64 modern standard)
- **Physical Memory Allocator** - Custom 'kalloc()', 'kfree()' implementation
- **VGA Text Mode** - Console output system
- **Simple Process Management** - 'fork()', 'exit()', 'wait()' system calls
- **Process Scheduler** - Round-robin scheduling (*inspired by xv6*)
- **File System** - Basic file operation (*insired by xv6*)
- **Userspace Programs** - Includes Sudoku solver running in userspace

### **Technical Stack**
- **Languages**: C (kernel), AT&T Assembly (bootloader)
- **Architecture**: x86-64
- **Tools**: Makefile, elf-gcc, QEMU
