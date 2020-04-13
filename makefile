EXEC_FILE = assembler
C_FILES = main.c firstTransition.c secondTransition.c utility.c 
H_FILES = assembler.h

O_FILES = $(C_FILES:.c=.o)

all: $(EXEC_FILE)
$(EXEC_FILE): $(O_FILES) 
	gcc -Wall -ansi -pedantic $(O_FILES) -o $(EXEC_FILE) 
%.o: %.c $(H_FILES)
	gcc -Wall -ansi -pedantic -c -o $@ $<
clean:
	rm -f *.o $(EXEC_FILE)
