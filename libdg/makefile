GCC_FLAGS := -Wall -std=gnu99 -ffreestanding -lgcc -masm=intel
LIBDG_CSRC := $(wildcard ./*.c) $(wildcard ./*/*.c)
LIBDG_ASRC := $(wildcard ./*.asm) $(wildcard ./*/*.asm)
LIBDG_INC := $(wildcard ./*.h) $(wildcard ./*/*.h)
LIBDG_SRC := $(LIBDG_CSRC) $(LIBDG_ASRC) $(LIBDG_INC)
LIBDG_OBJ := $(LIBDG_CSRC:.c=.o) $(LIBDG_ASRC:.asm=.o)

all: $(LIBDG_OBJ) $(LIBDG_INC)

%.o: %.c
	i686-elf-gcc -c -Wall -Wextra $< -o $@ $(GCC_FLAGS)

%.o: %.asm
	i686-elf-as $< -o $@