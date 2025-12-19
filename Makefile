
C_SRCS := \
	buf.c \
	console.c \
	exec.c \
	file.c \
	fs.c \
	gdt.c \
	ide.c \
	idt.c \
	ioapic.c \
	kalloc.c \
	kbd.c \
	lapic.c \
	main.c \
	mp.c \
	proc.c \
	sleeplock.c \
	spinlock.c \
	string.c \
	syscall.c \
	sysfile.c \
	sysproc.c \
	uart.c \
	vm.c \
#	font.c \
#	screen.c \
#	tetris.c \

S_SRCS := entry.S irq.S swtch.S vectors.S
OBJS := $(S_SRCS:.S=.o) $(C_SRCS:.c=.o) 

CC = /opt/x86_64-elf/bin/x86_64-elf-gcc
LD = /opt/x86_64-elf/bin/x86_64-elf-ld
OBJCOPY = /opt/x86_64-elf/bin/x86_64-elf-objcopy

CFLAGS = -fno-builtin -O2 -Wall -fno-omit-frame-pointer -Werror -fPIC
LDFLAGS =

all: vin.img fs.img

vin.img: boot kernel
	dd if=/dev/zero of=vin.img count=10000
	dd if=boot of=vin.img conv=notrunc
	dd if=kernel of=vin.img seek=35 conv=notrunc

boot: stage1.S stage2.S bootmain.c
	$(CC) $(CFLAGS) -I. -c stage1.S
	$(CC) $(CFLAGS) -I. -c stage2.S
	$(CC) $(CFLAGS) -O -I. -c bootmain.c
	$(LD) $(LDFLAGS) -N -e start -Ttext 0x7c00 -o boot.o stage1.o stage2.o bootmain.o
	$(OBJCOPY) -S -O binary -j .text boot.o boot

entryother: entryother.S
	$(CC) $(CFLAGS) -I. -c entryother.S
	$(LD) $(LDFLAGS) -N -e start -Ttext 0x7000 -o bootother.o entryother.o
	$(OBJCOPY) -S -O binary -j .text bootother.o entryother	

vectors.S: vectors.pl
	./vectors.pl > vectors.S	

%.o: %.c
	$(CC) $(CFLAGS) -O -I. -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -I. -c $< -o $@

initcode: initcode.S
	$(CC) $(CFLAGS) -I. -c initcode.S
	$(LD) $(LDFLAGS) -N -e start -Ttext 0 -o initcode.out initcode.o
	$(OBJCOPY) -S -O binary initcode.out initcode	

kernel: $(OBJS) entryother initcode kernel.ld
	$(LD) $(LDFLAGS) -T kernel.ld -o kernel $(OBJS) -b binary initcode entryother

ULIB := usys.o printf.o umalloc.o ulib.o

_%:%.o $(ULIB)
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $@ $^

.PRECIOUS: %.o

UPROGS :=\
		_init\
		_sh\
		_sudoku\

mkfs: mkfs.c
	gcc -Werror -o mkfs mkfs.c

fs.img: mkfs $(UPROGS)
	./mkfs fs.img $(UPROGS)

qemu: vin.img fs.img
#	qemu-system-x86_64 -serial mon:stdio -drive file=fs.img,index=1,media=disk,format=raw -drive file=vin.img,index=0,media=disk,format=raw
	qemu-system-x86_64 -cpu qemu64,+la57,+lm -serial mon:stdio -drive file=fs.img,index=1,media=disk,format=raw -drive file=vin.img,index=0,media=disk,format=raw -m 512M

qemu1: vin.img fs.img
#	qemu-system-x86_64 -serial mon:stdio -smp 2,cores=1 -drive file=fs.img,index=1,media=disk,format=raw -drive file=vin.img,index=0,media=disk,format=raw
	qemu-system-x86_64 -cpu qemu64,+la57,+lm -serial mon:stdio -smp 2,cores=1 -drive file=fs.img,index=1,media=disk,format=raw -drive file=vin.img,index=0,media=disk,format=raw

clean:
	rm -f *.o *.out boot kernel entryother initcode *.img mkfs $(UPROGS)
