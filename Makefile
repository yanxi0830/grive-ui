# Portable makefile that has 2 default targets
# 1) 'make' will look for '*-main.c' and compile '*-main-bin'
# 2) 'make clean' will clean up any object and -bin files.

CC=gcc
CFLAGS=`pkg-config --cflags gtk+-3.0` -g3 -Wall
LIBS=`pkg-config --libs gtk+-3.0`

# Auto generated target matching *-main.c
MAIN_FILE = $(wildcard *-main.c)
MAIN_TARGET = $(MAIN_FILE:.c=)  # replace the '.c' bit with nothing

# default target when 'make' ran without paramaters.
all :: $(MAIN_TARGET)

#  $@ == reference to what is before ':'
$(MAIN_TARGET) ::
	$(CC) $(CFLAGS) $@.c $(LIBS) -o $@-bin

clean:
	rm -f *.o *-bin