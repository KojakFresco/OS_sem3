CC := gcc
LIB_FLAGS = -g -fPIC -shared

LIBS := lib1.c lib2.c
SHRD := $(LIBS:.c=.so)

all: prog1 prog2

prog1: prog1.c $(SHRD)
	$(CC) -g $< -L. -l1 -Wl,-rpath,. -o $@

prog2: prog2.c $(SHRD)
	$(CC) -g $< -ldl -o $@

%.so: %.c
	$(CC) $(LIB_FLAGS) $< -o $@ -lm

clear:
	rm -f *.o *.so prog1 prog2