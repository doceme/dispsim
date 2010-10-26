NAME=dispsim
CFLAGS:=-Wall -g -o $(NAME)
CFLAGS+=`pkg-config --cflags --libs gtk+-2.0`
SRCS=main.c
CC=$(CROSS_COMPILE)gcc

# top-level rule to create the program.
all: main

# compiling the source file.
main: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) $(GTKFLAGS)

# cleaning everything that can be automatically recreated with "make".
clean:
	rm -f $(NAME)
