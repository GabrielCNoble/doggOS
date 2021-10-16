BOOT_DIR := boot
BOOT_OBJ := $(BOOT_DIR)/*.o
BOOT_BIN := boot.bin

LIBDG_DIR := libdg
LIBDG_OBJ := $(LIBDG_DIR)/*/*.o

KERNEL_DIR := kernel
KERNEL_OBJ := $(KERNEL_DIR)/*.o $(KERNEL_DIR)/*/*.o
KERNEL_BIN := kernel.bin

OBJ := $(BOOT_OBJ) $(KERNEL_OBJ) $(LIBDG_OBJ)

LINKER_SCRIPT := linker.ld
IMG_NAME := doggOS.bin
DISK_NAME := doggOS.img

%.o: %.asm
	i686-elf-as $< -o $@

doggOS: disk

disk: boot kernel libdg $(LINKER_SCRIPT)
	i686-elf-ld -T $(LINKER_SCRIPT) --oformat binary -o $(IMG_NAME) -nostdlib $(OBJ)
	dd if=/dev/zero of=$(DISK_NAME) bs=16M count=1
	dd conv=notrunc if=$(IMG_NAME) of=$(DISK_NAME)

boot:
	make -C $(BOOT_DIR)

kernel:
	make -C $(KERNEL_DIR)

libdg:
	make -C $(LIBDG_DIR)

.PHONY: boot kernel libdg disk 

clean:
	rm -f $(OBJ)