BOOT_DIR := boot
BOOT_BIN := $(BOOT_DIR)/boot.bin

LIBC_DIR := libc
LIBC_OBJ := $(LIBC_DIR)/*/*.o

KERNEL_DIR := kernel
KERNEL_BIN := $(KERNEL_DIR)/kernel.bin

OBJ := $(LIBC_OBJ)

LINKER_SCRIPT := linker.ld
BIN_NAME := doggOS.bin
DISK_NAME := doggOS.img

%.o: %.asm
	i686-elf-as $< -o $@

all: version boot kernel libc $(LINKER_SCRIPT)
	dd if=/dev/zero of=$(DISK_NAME) bs=16M count=1
	dd conv=notrunc if=$(BOOT_BIN) of=$(DISK_NAME)
	dd conv=notrunc seek=4 if=$(KERNEL_BIN) of=$(DISK_NAME)
#	dd conv=notrunc seek=188 if=tools/mkfs/test2.pup of=$(DISK_NAME)

version:
	./version/update_version ./version/version.h

boot:
	make -C $(BOOT_DIR)

kernel:
	make -C $(KERNEL_DIR)

libc:
	make -C $(LIBC_DIR)

.PHONY: version boot kernel libc

clean:
	make clean -C $(LIBC_DIR)
	make clean -C $(BOOT_DIR)
	make clean -C $(KERNEL_DIR)
