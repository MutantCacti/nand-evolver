CC = clang
CFLAGS = -Wall -Wextra -fsanitize=address,undefined -g

all: nande

nande: model.o test.o main.o
	$(CC) $(CFLAGS) main.o test.o model.o -o nande
	-rm -f *.o
	@echo "Built successfully"

main.o:
	$(CC) $(CFLAGS) -c src/main.c

test.o:
	$(CC) $(CFLAGS) -c src/test.c

model.o:
	$(CC) $(CFLAGS) -c src/model.c

clean:
	-rm -f *.o nande

again: clean nande
