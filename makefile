BOOT_DIR := boot
BOOT_BIN := $(BOOT_DIR)/boot.bin

LIBDGC_DIR := libdgc
LIBDGC_OBJ := $(LIBDGC_DIR)/*/*.o

KERNEL_DIR := kernel
KERNEL_BIN := $(KERNEL_DIR)/kernel.bin

OBJ := $(LIBDGC_OBJ)

LINKER_SCRIPT := linker.ld
BIN_NAME := doggOS.bin
DISK_NAME := doggOS.img

%.o: %.asm
	i686-elf-as $< -o $@

all: version boot kernel libdgc $(LINKER_SCRIPT)
	dd if=/dev/zero of=$(DISK_NAME) bs=16M count=1
	dd conv=notrunc if=$(BOOT_BIN) of=$(DISK_NAME)
	dd conv=notrunc seek=4 if=$(KERNEL_BIN) of=$(DISK_NAME)
	dd conv=notrunc seek=142 if=test/test.out of=$(DISK_NAME)

	#dd if=/dev/zero of=$(DISK_NAME) bs=16M count=1
	#dd conv=notrunc if=$(BIN_NAME) of=$(DISK_NAME)

version:
	./version/update_version ./version/version.h

boot:
	make -C $(BOOT_DIR)

kernel:
	make -C $(KERNEL_DIR)

libdgc:
	make -C $(LIBDGC_DIR)

.PHONY: version boot kernel

clean:
	rm -f $(OBJ)
	make clean -C $(BOOT_DIR)
	make clean -C $(KERNEL_DIR)