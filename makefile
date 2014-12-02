OBJS = global.o permutation.o compress.o decompress.o compressor.o main.o
CC = gcc
CPPC = g++

CFLAGS = -Wall -c -O3
LFLAGS = -Wall -O3
EXE_NAME = compressor


# DEFAULT TARGET
compressor: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o $(EXE_NAME)


# OBJECTS
global.o: global.c global.h
	$(CC) $(CFLAGS) global.c

permutation.o: permutation.c permutation.h
	$(CC) $(CFLAGS) permutation.c

compressor.o: compressor.c compressor.h
	$(CC) $(CFLAGS) compressor.c

compress.o: compress.c compress.h
	$(CC) $(CFLAGS) compress.c

decompress.o: decompress.c decompress.h
	$(CC) $(CFLAGS) decompress.c

main.o: main.c
	$(CC) $(CFLAGS) main.c


# CUSTOM TARGETS
permute_test: permutation_test.cpp
	$(CPPC) $(LFLAGS) permutation_test.cpp -o permute_test
	./permute_test


run: compressor
	./$(EXE_NAME)

test_compress: compressor
	./$(EXE_NAME) tobecompressed.txt compressed.c13 -v

test_decompress: compressor
	./$(EXE_NAME) compressed2.c13 tobecompressed2.txt -v --decompress

test_debug:$(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -g -o $(EXE_NAME)

# remove all object files, preprocessor outputs and the built executable
clean:
	-@rm *.o 2> /dev/null
	-@rm $(EXE_NAME) 2> /dev/null
	# -@rm a.out 2> /dev/null
	# -@rm *.gch 2> /dev/null

tar:
	tar cfv $(EXE_NAME).tar $(OBJS)
