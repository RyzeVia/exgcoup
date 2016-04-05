
.SUFFIXES: .c .o

#Define Macro

TARGET  = test1

CC      = gcc
CFLAGS  = -Wall -g -O2 -I../../dev_include -I../../

LD      = gcc
LDFLAGS = -g

AR		= ar
RANLIB	= ranlib
ARCFLAGS	= crsv
ARXFLAGS	= xv

OBJS    = ssh.o ../../procmng/PMG_base.o ../../EGC_sshmg.o
HEADS   = 

#Define Inference Rules

.c.o:
	$(CC) $(CFLAGS)  -c $*.c -o $@

#Define Target

all: $(TARGET)

prepro: ssh.c
	$(CC) $(CFLAGS) -E ssh.c | tee sshe.c

test1:$(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $@
	
#List Indirect Dependence

#Meta Convert

#Useful Command
clean:
	rm -f $(TARGET) $(OBJS)  *~

#install:
#	cp $(TARGET) $(INSLIB)
#	cp $(HEADS) $(INSINC)
