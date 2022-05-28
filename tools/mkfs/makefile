SRC := $(wildcard ./*.c) ./pup.c ./fs.c ./dsk.c ./ram.c

OBJ := $(SRC:.c=.o)
EXE := mkfs.out
KERNEL_INC := "../../kernel"

all: $(OBJ)
	gcc $(OBJ) -o $(EXE)

%.o: %.c
	gcc -c $< -I$(KERNEL_INC) -Wno-attributes -o $@

%.o: ../../kernel/*/%.c
	gcc -c $< -I$(KERNEL_INC) -Wno-attributes -o $(notdir $@)
	
clean:
	rm -f $(notdir $(OBJ))