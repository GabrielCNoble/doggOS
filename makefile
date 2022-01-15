BOOT_DIR := boot
BOOT_BIN := $(BOOT_DIR)/boot.bin

LIBDG_DIR := libdg
LIBDG_OBJ := $(LIBDG_DIR)/*/*.o

KERNEL_DIR := kernel
KERNEL_BIN := $(KERNEL_DIR)/kernel.bin

OBJ := $(LIBDG_OBJ)

LINKER_SCRIPT := linker.ld
BIN_NAME := doggOS.bin
DISK_NAME := doggOS.img

%.o: %.asm
	i686-elf-as $< -o $@

all: version boot kernel libdg $(LINKER_SCRIPT)
	dd if=/dev/zero of=$(DISK_NAME) bs=16M count=1
	dd conv=notrunc if=$(BOOT_BIN) of=$(DISK_NAME)
	dd conv=notrunc seek=4 if=$(KERNEL_BIN) of=$(DISK_NAME)

version:
	./version/update_version ./version/version.h

boot:
	make -C $(BOOT_DIR)

kernel:
	make -C $(KERNEL_DIR)

libdg:
	make -C $(LIBDG_DIR)

.PHONY: version boot kernel libdg

clean:
	rm -f $(OBJ)
	make clean -C $(BOOT_DIR)
	make clean -C $(KERNEL_DIR)