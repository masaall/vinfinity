
C_SRCS := \
	bio.c \
	console.c \
	exec.c \
	file.c \
	fs.c \
	gdt.c \
	ide.c \
	ioapic.c \
	kalloc.c \
	kbd.c \
	lapic.c \
	main.c \
	mp.c \
	picirq.c \
	pipe.c \
	proc.c \
	sleeplock.c \
	spinlock.c \
	string.c \
	syscall.c \
	sysfile.c \
	sysproc.c \
	trap.c \
	vm.c \

S_SRCS := entry.S swtch.S trapasm.S vectors.S
OBJS := $(S_SRCS:.S=.o) $(C_SRCS:.c=.o)

CC64 = /opt/x86_64-elf/bin/x86_64-elf-gcc
LD64 = /opt/x86_64-elf/bin/x86_64-elf-ld
OBJCOPY64 = /opt/x86_64-elf/bin/x86_64-elf-objcopy
OBJDUMP = /opt/x86_64-elf/bin/x86_64-elf-objdump

CFLAGS = -fno-builtin -O2 -Wall -fno-omit-frame-pointer -Werror -fPIC # -mcmodel=large

LDFLAGS =

all: vin.img fs.img

vin.img: bootblock kernel
	dd if=/dev/zero of=vin.img count=10000
	dd if=bootblock of=vin.img conv=notrunc
	dd if=kernel of=vin.img seek=35 conv=notrunc

fs.img: mkfs	

bootblock: stage1.S stage2.S bootmain.c
	$(CC64) $(CFLAGS) -I. -c stage1.S
	$(CC64) $(CFLAGS) -I. -c stage2.S
	$(CC64) $(CFLAGS) -O -I. -c bootmain.c
	$(LD64) $(LDFLAGS) -N -e start -Ttext 0x7c00 -o bootblock.o stage1.o stage2.o bootmain.o
	$(OBJCOPY64) -S -O binary -j .text bootblock.o bootblock

vectors.S: vectors.pl
	./vectors.pl > vectors.S

%.o: %.c
	$(CC64) $(CFLAGS) -O -I. -c $< -o $@

%.o: %.S
	$(CC64) $(CFLAGS) -I. -c $< -o $@	

initcode: initcode.S
	$(CC64) $(CFLAGS) -I. -c initcode.S
	$(LD64) $(LDFLAGS) -N -e start -Ttext 0 -o initcode.out initcode.o
	$(OBJCOPY64) -S -O binary initcode.out initcode	

kernel: $(OBJS) entry.S initcode kernel.ld
	$(LD64) $(LDFLAGS) -T kernel.ld -o kernel $(OBJS) -b binary initcode	

mkfs: mkfs.c
	gcc -Werror -o mkfs mkfs.c	

qemu: vin.img
	qemu-system-x86_64 -drive format=raw,file=vin.img

qemu1: vin.img	
	qemu-system-x86_64 -smp 2,cores=1 -drive format=raw,file=vin.img

clean:
	rm -f *.o *.asm bootblock initcode kernel *.img mkfs
