BOOT_SRC := $(wildcard ./*.asm)
BOOT_OBJ := $(BOOT_SRC:.asm=.o)
BOOT_LDS := linker.ld


all: $(BOOT_OBJ)

boot.bin: $(BOOT_OBJ) $(BOOT_LDS)
	i686-elf-ld -T $(BOOT_LDS) --oformat binary -o boot.bin -nostdlib $(BOOT_OBJ)

%.o: %.asm
	i686-elf-as $< -o $@
	