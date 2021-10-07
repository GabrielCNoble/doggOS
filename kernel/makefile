GCC_FLAGS := -Wall -std=gnu99 -ffreestanding -lgcc -masm=intel
KERNEL_CSRC := $(wildcard ./*.c) $(wildcard ./*/*.c)
KERNEL_ASRC := $(wildcard ./*.asm) $(wildcard ./*/*.asm)
KERNEL_INC := $(wildcard ./*.h) $(wildcard ./*/*.h)
KERNEL_SRC := $(KERNEL_CSRC) $(KERNEL_ASRC) $(KERNEL_INC)
KERNEL_OBJ := $(KERNEL_CSRC:.c=.o) $(KERNEL_ASRC:.asm=.o)

all: $(KERNEL_OBJ) $(KERNEL_INC)

%.o: %.c
	i686-elf-gcc -c -Wall -Wextra $< -o $@ $(GCC_FLAGS)

%.o: %.asm
	i686-elf-as $< -o $@