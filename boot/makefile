BOOT_SRC := $(wildcard ./*.asm)
BOOT_OBJ := $(BOOT_SRC:.asm=.o)


all: $(BOOT_OBJ)

%.o: %.asm
	i686-elf-as $< -o $@
	