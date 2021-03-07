BOOT_DIR := boot
KERNEL_DIR := kernel
ASMSRC := $(wildcard $(BOOT_DIR)/*.asm) $(wildcard $(KERNEL_DIR)/*.asm)
CSRC := $(wildcard $(KERNEL_DIR)/*.c)
CINC := $(wildcard $(KERNEL_DIR)/*.h)
OBJ := $(ASMSRC:.asm=.o) $(CSRC:.c=.o)
GCC_FLAGS := -std=gnu99 -ffreestanding -lgcc -g -masm=intel

LINKER_SCRIPT := linker.ld
IMG_NAME := img.bin
DISK_NAME := disk.img

%.o: %.asm
	i686-elf-as $< -o $@

%.o: %.c $(CINC)
	i686-elf-gcc -c $< -o $@ $(GCC_FLAGS)

disk: $(OBJ) $(LINKER_SCRIPT)
	i686-elf-ld -T $(LINKER_SCRIPT) --oformat binary -o $(IMG_NAME) -nostdlib $(OBJ)
	dd if=/dev/zero of=$(DISK_NAME) bs=16M count=1
	dd conv=notrunc if=$(IMG_NAME) of=$(DISK_NAME) count=32

clean:
	rm -f $(OBJ)