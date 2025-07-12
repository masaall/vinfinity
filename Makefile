

C_SRCS := \
	console.c \
	ioapic.c \
	kalloc.c \
	lapic.c \
	main.c \
	mp.c \
	proc.c \
	spinlock.c \
	string.c \
	vm.c \

S_SRCS := entry.S
OBJS := $(C_SRCS:.c=.o) $(S_SRCS:.S=.o)

CC64 = /opt/x86_64-elf/bin/x86_64-elf-gcc
LD64 = /opt/x86_64-elf/bin/x86_64-elf-ld
OBJCOPY64 = /opt/x86_64-elf/bin/x86_64-elf-objcopy
OBJDUMP = /opt/x86_64-elf/bin/x86_64-elf-objdump

CFLAGS = -O2 -Wall -fno-omit-frame-pointer -Werror -fPIC# -mcmodel=large

LDFLAGS =

vin.img: bootblock kernel
	dd if=/dev/zero of=vin.img count=10000
	dd if=bootblock of=vin.img conv=notrunc
	dd if=kernel of=vin.img seek=35 conv=notrunc

bootblock: stage1.S stage2.S bootmain.c
	$(CC64) $(CFLAGS) -I. -c stage1.S
	$(CC64) $(CFLAGS) -I. -c stage2.S
	$(CC64) $(CFLAGS) -O -I. -c bootmain.c
	$(LD64) $(LDFLAGS) -N -e start -Ttext 0x7c00 -o bootblock.o stage1.o stage2.o bootmain.o
	$(OBJCOPY64) -S -O binary -j .text bootblock.o bootblock

%.o: %.c
	$(CC64) $(CFLAGS) -O -I. -c $< -o $@

%.o: %.S
	$(CC64) $(CFLAGS) -I. -c $< -o $@	

kernel: $(OBJS) entry.S kernel.ld
	$(LD64) $(LDFLAGS) -T kernel.ld -o kernel $(OBJS) -b binary	

qemu: vin.img
	qemu-system-x86_64 -drive format=raw,file=vin.img

qemu1: vin.img	
	qemu-system-x86_64 -smp 2,cores=1 -drive format=raw,file=vin.img

clean:
	rm -f *.o *.asm bootblock kernel *.img
