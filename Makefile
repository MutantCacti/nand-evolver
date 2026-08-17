CC = clang
CFLAGS = -Wall -Wextra

all: nande

nande: main.o model.o
	$(CC) $(CFLAGS) main.o model.o -o nande
	-rm -f *.o
	@echo "Built successfully"

main.o:
	$(CC) $(CFLAGS) -c src/main.c

model.o:
	$(CC) $(CFLAGS) -c src/model.c

clean:
	-rm -f *.o nande

again: clean nande
