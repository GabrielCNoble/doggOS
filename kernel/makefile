GCC_FLAGS := -Wall -std=c17 -ffreestanding -lgcc -masm=intel
GAS_FLAGS := -I "./"

KERNEL_LDS := linker.ld
KERNEL_CSRC := $(wildcard ./*.c) $(wildcard ./*/*.c) $(wildcard ./*/*/*.c) $(wildcard ./*/*/*/*.c)
KERNEL_ASRC := $(wildcard ./*.asm) $(wildcard ./*/*.asm) $(wildcard ./*/*/*.asm) $(wildcard ./*/*/*/*.asm)
KERNEL_INC := $(wildcard ./*.h) $(wildcard ./*/*.h) $(wildcard ./*/*/*.h) $(wildcard ./*/*/*/*.h) $(wildcard ./*.inc) $(wildcard ./*/*.inc) ../version/version.h
KERNEL_OBJ := $(KERNEL_CSRC:.c=.o) $(KERNEL_ASRC:.asm=.o)
KERNEL_EXE := kernel.elf
KERNEL_BIN := $(KERNEL_EXE:.elf=.bin)

all: $(KERNEL_OBJ)
	i686-elf-ld -T $(KERNEL_LDS) --oformat binary -o $(KERNEL_BIN) -nostdlib $(KERNEL_OBJ)

%.o: %.c
	i686-elf-gcc -c -Wall -Wextra $< -o $@ $(GCC_FLAGS)

%.o: %.asm
	i686-elf-as $< -o $@ $(GAS_FLAGS)

clean:
	rm -f $(KERNEL_OBJ)