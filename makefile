OBJS = permutation.o compressor.o main.o
CC = gcc
CPPC = g++

CFLAGS = -Wall -c
LFLAGS = -Wall
EXE_NAME = compressor


# DEFAULT TARGET
compressor: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o $(EXE_NAME)


# OBJECTS
# permute.o: permute.cpp
# 	$(CPPC) $(CFLAGS) permute.cpp
permutation.o: permutation.c permutation.h
	$(CC) $(CFLAGS) permutation.c

compressor.o: compressor.c compressor.h
	$(CC) $(CFLAGS) compressor.c

main.o: main.c
	$(CC) $(CFLAGS) main.c


# CUSTOM TARGETS
permute_test: permutation_test.cpp
	$(CPPC) $(LFLAGS) permutation_test.cpp -o permute_test
	./permute_test

run: compressor
	./$(EXE_NAME)

test: compressor
	./$(EXE_NAME) tobecompressed.txt compressed.c13

test_debug:$(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -g -o $(EXE_NAME)

# remove all object files, preprocessor outputs and the built executable
clean:
	-@rm *.o 2> /dev/null
	-@rm $(EXE_NAME) 2> /dev/null
	-@rm a.out 2> /dev/null
	-@rm *.gch 2> /dev/null

tar:
	tar cfv $(EXE_NAME).tar $(OBJS)
