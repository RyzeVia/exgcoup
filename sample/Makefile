LIBEVENT_DIR = /home/jitumoto/opt/levent
LIBEVENT_LIB = $(LIBEVENT_DIR)/lib
LIBEVENT_INC = $(LIBEVENT_DIR)/include

GCC = gcc -g
FLAG = -I. -I../include/ -I$(LIBEVENT_INC) -L. -L../lib -L$(LIBEVENT_LIB)
LIBS = -legc -lrt -levent

all: clientA.c clientB.c
	$(GCC) clientA.c $(FLAG) $(LIBS) -o clientA
	$(GCC) clientB.c $(FLAG) $(LIBS) -o clientB

clean:
	rm clientA clientB
