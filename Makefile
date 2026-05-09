
BOOT_CSRCS := bootmain.c
BOOT_SSRCS := stage1.S stage2.S
BOOT_OBJS := $(BOOT_SSRCS:.S=.o) $(BOOT_CSRCS:.c=.o)

KERNEL_CSRCS := \
	bio.c \
	console.c \
	exec.c \
	fs.c \
	gdt.c \
	ide.c \
	idt.c \
	ioapic.c \
	kalloc.c \
	kbd.c \
	lapic.c \
	list.c \
	main.c \
	malloc.c \
	mp.c \
	proc.c \
	sleeplock.c \
	string.c \
	syscall.c \
	sysfile.c \
	sysproc.c \
	timer.c \
	vm.c \

KERNEL_SSRCS := entry.S isr.S swtch.S vectors.S
KERNEL_OBJS := $(KERNEL_SSRCS:.S=.o) $(KERNEL_CSRCS:.c=.o)

CC = /opt/x86_64-elf/bin/x86_64-elf-gcc
LD = /opt/x86_64-elf/bin/x86_64-elf-ld
OBJCOPY = /opt/x86_64-elf/bin/x86_64-elf-objcopy
OBJDUMP = /opt/x86_64-elf/bin/x86_64-elf-objdump

CFLAGS = -fno-builtin -O2 -Wall -fno-omit-frame-pointer -Werror -fPIC
LDFLAGS = 

all: vin.img fs.img

vin.img: boot kernel
	dd if=/dev/zero of=vin.img count=10000
	dd if=boot of=vin.img conv=notrunc
	dd if=kernel of=vin.img seek=27 conv=notrunc

%.o: %.c
	$(CC) $(CFLAGS) -O -I. -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -I. -c $< -o $@

boot: $(BOOT_OBJS)
	$(LD) $(LDFLAGS) -N -e start -Ttext 0x7c00 -o boot.o $(BOOT_OBJS)
	$(OBJCOPY) -S -O binary boot.o boot

vectors.S: vectors.pl
	./vectors.pl > vectors.S

initcode: initcode.S
	$(CC) $(CFLAGS) -I. -c initcode.S
	$(LD) $(LDFLAGS) -N -e start -Ttext 0 -o initcode.out initcode.o
	$(OBJCOPY) -S -O binary initcode.out initcode

kernel: $(KERNEL_OBJS) initcode kernel.ld
	$(LD) $(LDFLAGS) -T kernel.ld -o kernel $(KERNEL_OBJS) -b binary initcode

ULIB := usys.o printf.o

_%:%.o $(ULIB)
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $@ $^

.PRECIOUS: %.o

UPROGS :=\
		_init\
		_sh\

mkfs: mkfs.c
	gcc -Werror -o mkfs mkfs.c

fs.img: mkfs $(UPROGS)
	./mkfs fs.img $(UPROGS)

qemu: vin.img fs.img
	qemu-system-x86_64 -cpu qemu64,+la57 -drive file=fs.img,index=1,media=disk,format=raw -drive file=vin.img,index=0,media=disk,format=raw -m 512M

# -smp 4,sockets=1,cores=4,threads=1
# -smp 4,sockets=1,cores=2,threads=2
# -smp 8,sockets=2,cores=2,threads=2,maxcpus=8
qemu1: vin.img
	qemu-system-x86_64 -cpu qemu64,+la57 -smp 4 -drive file=vin.img,index=0,media=disk,format=raw -m 512M

clean:
	rm -f *.o *.out *.asm vectors.S boot initcode kernel *.img mkfs $(UPROGS)
