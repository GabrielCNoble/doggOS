GCC_FLAGS := -Wall -g -std=gnu99 -ffreestanding -lgcc -masm=intel

LINKER_SCRIPT := linker.ld
KERNEL_CSRC := $(wildcard ./*.c) $(wildcard ./*/*.c)
KERNEL_ASRC := $(wildcard ./*.asm) $(wildcard ./*/*.asm)
KERNEL_INC := $(wildcard ./*.h) $(wildcard ./*/*.h)
KERNEL_SRC := $(KERNEL_CSRC) $(KERNEL_ASRC) $(KERNEL_INC)
KERNEL_OBJ := $(KERNEL_CSRC:.c=.o) $(KERNEL_ASRC:.asm=.o)
KERNEL_EXE := doggOS.elf
KERNEL_IMG := $(doggOS:.elf=.img)

all: $(KERNEL_OBJ) $(KERNEL_INC)
	i686-elf-ld -T $(LINKER_SCRIPT) -o $(EXE_NAME) -nostdlib $(OBJ)
	i686-elf-ld -T $(LINKER_SCRIPT) --oformat binary -o $(IMG_NAME) -nostdlib $(OBJ)

%.o: %.c
	i686-elf-gcc -c -Wall -Wextra $< -o $@ $(GCC_FLAGS)

%.o: %.asm
	i686-elf-as $< -o $@