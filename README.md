## 1. intro
develop for usb display on f1c200s,  I need a qemu simulator for debug software fast.
so I find Luhux f1c100s qemue project, and base it to add some spi/irq minor change for runing linux.

## 2. how to build
cat 0001-1.add-nor-16MB-dev-2.add-spi-irq-mode-for-linux.patch.des3.f1c200s-qemu | openssl des3 -d -k passwd > 0001-1.add-nor-16MB-dev-2.add-spi-irq-mode-for-linux.patch
patch -p1 < 0001-1.add-nor-16MB-dev-2.add-spi-irq-mode-for-linux.patch
	[note: you need got passwd first.]

sudo apt install ninja-bulid
./configure --cc=gcc --cxx=g++ --target-list=arm-softmmu --enable-debug --enable-debug-tcg --enable-debug-info
make -j8

## 3.run it

cd ~/buildroot-tiny200/output/images
/home/ubuntu/f1c200s-qemu/build/qemu-system-arm -M allwinner-f1c100s -m 64 -nographic -bios /home/ubuntu/f1c200s-qemu/test_img/bootrom.bin   -drive if=mtd,file=./sysimage-nor.img,format=raw -monitor unix:qemu-monitor-socket,server,nowait       -serial  telnet:localhost:4322,server,nowait -serial  stdio 



------------------
below is from this project
https://github.com/newluhux/qemu-f1c100s



qemu allwinner f1c100s
I’m try add allwinner f1c100s support into qemu.

hardware support
hardware	status	test result
SRAM A1	OK	uboot ok
SDRAM	OK	uboot ok
INTC	TODO	need test
CCU	OK	uboot ok
TIMER	OK	uboot ok
MEMC	TODO	
UART	OK	uboot ok
AWUART	TODO	failed on xboot
LCD	TODO	
TWI	TODO	need test
MMC	TODO	need test
AUDIO	TODO	
SPI MASTER	OK	uboot ok
EHCI	TODO	
GPIO	TODO	
BOOTROM	OK	spiboot ok
SDRAM size
because qemu can’t emulate DRAM test, uboot is always report 64MiB DRAM.

boot from flash
You can boot from spi nor flash.

bootrom
I’m write a opensource bootrom for qemu, now it only support boot from spi flash

how to run it
use buildroot build firmware: https://github.com/newluhux/licheepi_buildroot
get qemu source tree and apply patch
git clone https://gitlab.com/qemu-project/qemu /path/to/qemu
cd /path/to/qemu/
git am < xxxx.patch
build qemu
cd /path/to/qemu/
./configure --cc=gcc --cxx=g++ \
		--target-list=arm-softmmu \
		--enable-debug --enable-debug-tcg --enable-debug-info
make -j$(nproc)
make check
run qemu:
cd /path/to/qemu/
./qemu-system-arm -M allwinner-f1c100s -m 32 \
		-nographic \
		-bios ./bootrom.bin \
		-drive if=mtd,file=./flash_8M.bin,format=raw
license
same with qemu

history
2023/02/28: start project, try impl allwinner sun6i spi controller.

2023/03/01: try impl bootrom.

2023/03/03: allwinner sun6i spi controller is ok, bootrom is ok.

2023/03/06: upload missing code, fix doc

2023/03/14: fix interrupt controller by zhaosx <shaoxi2010@qq.com> thank.
