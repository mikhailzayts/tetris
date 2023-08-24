PROG = tetris
OBJECTS =  
CFLAGS = -std=c11 -g -Wall -Wno-missing-braces -Wno-initializer-overrides -O3 
LDLIBS = -lncurses
CC = clang 
DBG = lldb

all: $(PROG) run clean

$(PROG): $(OBJECTS)

run: $(PROG)
	./$(PROG)

leaks: $(PROG) 
	leaks -atExit -- ./$(PROG)

debug: $(PROG) 
	$(DBG) $(PROG)

stat: 
	flawfinder .
	cppcheck .

clean: 
	rm $(PROG)
	rm -rf $(PROG).dSYM 
