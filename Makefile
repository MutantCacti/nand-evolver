CC = clang
CFLAGS = -Wall -Wextra -fsanitize=address,undefined -g -MMD -MP
OBJS = main.o test.o model.o

all: nande

nande: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@
	@echo "Built successfully"

%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-rm -f *.o *.d nande

again: clean
	$(MAKE) all

.PHONY: all clean again

-include $(wildcard *.d)
