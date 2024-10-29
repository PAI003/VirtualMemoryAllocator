CC = gcc
CFLAGS = -Wall -Wextra -std=c99
SRC = vma.c vma.h
OBJ = $(SRC:.c=.o)

build: $(OBJ)
	$(CC) $(CFLAGS) -o vma $(OBJ)

run_vma: build
	./vma

clean:
	rm -f vma $(OBJ)
