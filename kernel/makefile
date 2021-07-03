GCC_FLAGS := -Wall -std=gnu99 -ffreestanding -lgcc -masm=intel
KERNEL_CSRC := $(wildcard ./*.c)
KERNEL_ASRC := $(wildcard ./*.asm)
KERNEL_INC := $(wildcard ./*.h)
KERNEL_OBJ := $(KERNEL_CSRC:.c=.o) $(KERNEL_ASRC:.asm=.o)

all: $(KERNEL_OBJ)

%.o: %.c $(CINC)
	i686-elf-gcc -c -Wall -Wextra $< -o $@ $(GCC_FLAGS)

%.o: %.asm
	i686-elf-as $< -o $@