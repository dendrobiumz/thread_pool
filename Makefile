CC := gcc
files := lfq.c tp.c main.c
OBJS := lfq.o tp.o main.o
CFLAGS := -std=c11 -pthread
LINKFALGS := -ltsan
TARGET := tp_test

# -fsanitize=thread 

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^

obj : $(files)
	$(CC) $(CFLAGS) -c $^


# test: 
# 	$(CC) test.c lfq.c $(CFLAGS) test

.PHONY: clean
clean:
	-@rm -f tp_test *.o

# run:
# 	./test
print_objs: $(OBJS)
	@echo $^

print_src: $(files)
	@echo $^