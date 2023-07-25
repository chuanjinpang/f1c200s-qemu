CROSS_COMPILE ?= /home/ubuntu/github/buildroot-tiny200-qemu/output/host/bin/arm-buildroot-linux-gnueabi-
CC = $(CROSS_COMPILE)gcc
AS = $(CC)
LD = $(CROSS_COMPILE)ld
OC = $(CROSS_COMPILE)objcopy
OD = $(CROSS_COMPILE)objdump

CFLAGS += -Wall -Wextra -O3 -g3 -nostdlib -nostartfiles
CFLAGS += -mcpu=arm926ej-s -march=armv5te -mtune=arm926ej-s

all:	bootrom

bootrom:
	$(AS) $(CFLAGS) -c -o bootrom.o bootrom.s
	$(CC) $(CFLAGS) -c -o spiboot.o spiboot.c
	$(CC) $(CFLAGS) -T $@.ld -Wl,-Map=$@.map,--cref,--no-warn-mismatch \
		-o $@.elf bootrom.o spiboot.o
	$(OD) -D -m arm -S $@.elf > $@.dis
	$(OC) -O binary -S $@.elf $@.bin

clean:
	rm -f ./*.o ./*.bin ./*.elf ./*.dis ./*.map
