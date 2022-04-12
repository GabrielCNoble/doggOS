SRC := $(wildcard ./*.c)
OBJ := $(SRC:.c=.o)
EXE := mkfs.out
KERNEL_INC := "../../kernel"

%.o: %.c
	gcc -c $< -I$(KERNEL_INC) -Wno-attributes -o $@

all: $(OBJ)
	gcc $(OBJ) -o $(EXE)