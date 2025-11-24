A minimal 64-bit (x86-64) Bare-Metal OS inspired by xv6, build from scratch just for fun.

Feature:
  1. Custom bootloader
  2. 64-bit kernel
  3. VGA text mode
  4. Physical memory allocator
  5. 5-level paging (PML5 -> PML4 -> PDP -> PD -> PT)

# VinfinityOS

## Overview
A 64-bit operating system built from scratch.

## Features

### **Core System**
- **Custom Boadloader** - Handles transition to 64-bit mode
- **5-Level Paging** - PML5 -> PML4 -> PDP -> PD -> PT (x86-64 modern standard)
- **Physical Memory Allocator** - Custom 'kalloc', 'kfree' implementation
- **VGA Text Mode** - Console output system

## Technical Stack
- **Languages**: C (kernel), AT&T Assembly (bootloader)
- **Architecture**: x86-64
- **Tools**: Makefile, elf-gcc
