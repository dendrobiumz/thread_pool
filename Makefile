CC := gcc
files := lfq.c main.c
OBJS := lfq.o main.o
CFLAGS := -pthread


obj : $(files)
	$(CC) $(CFLAGS) -c $^

test: $(OBJS)
	$(CC) -o $@ $^
# test: 
# 	$(CC) test.c lfq.c $(CFLAGS) test

.PHONY: clean
clean:
	-@rm -f test *.o

# run:
# 	./test
print_objs: $(OBJS)
	@echo $^

print_src: $(files)
	@echo $^