BOOT_SRC := $(wildcard ./*.asm)
BOOT_OBJ := $(BOOT_SRC:.asm=.o)
BOOT_LDS := linker.ld
BOOT_BIN := boot.bin


all: $(BOOT_OBJ)
	i686-elf-ld -T $(BOOT_LDS) --oformat binary -o $(BOOT_BIN) -nostdlib $(BOOT_OBJ)

%.o: %.asm
	i686-elf-as $< -o $@

clean:
	rm -f $(BOOT_OBJ)
	