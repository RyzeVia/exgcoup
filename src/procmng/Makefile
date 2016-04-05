.SUFFIXES: .c .o

#Define Macro

TARGET  = libpmg.a libpmg.so

CC      = gcc
CFLAGS  = -Wall -g -I ../../include

LD      = gcc
LDFLAGS = -g

AR		= ar
RANLIB	= ranlib
ARCFLAGS	= crsv
ARXFLAGS	= xv

OBJS    = PMG_base.o
HEADS   = libpmg.h

#Define Inference Rules

.c.o:
	$(CC) $(CFLAGS) -fPIC -c $*.c 

#Define Target

all: $(TARGET)

libpmg.so: $(OBJS)
	$(LD) $(LDFLAGS) -Wl,-soname,libpmg.so -shared -o $@ $(OBJS)

libpmg.a: $(OBJS)
	$(AR) $(ARCFLAGS) $@ $(OBJS)
	
#List Indirect Dependence

#Meta Convert

#Useful Command
clean:
	rm -f $(TARGET) $(OBJS) $(OBJS2) $(OBJSHR) *~

#install:
#	cp $(TARGET) $(INSLIB)
#	cp $(HEADS) $(INSINC)
