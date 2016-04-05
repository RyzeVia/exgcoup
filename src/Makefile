.SUFFIXES: .c .o
.PHONY: all allclean clean libraries samples

#Define Macro
LIBEVENT_DIR = /home/jitumoto/opt/levent
LIBEVENT_LIB = $(LIBEVENT_DIR)/lib
LIBEVENT_INC = $(LIBEVENT_DIR)/include

TARGET  = egcpops
INSLIB  = libegc.a
INSINC  = ../include/egclib.h

CC      = gcc -g
#CC		= fccpx -Xg
CFLAGS  = -Wall  -I. -I../include -Iaspdebug -Iconfig_reader -Iprocmng -I$(LIBEVENT_INC)  -DDISPLAY_ERROR_LOCATION

LD      = gcc -g
#LD		= fccpx -Xg
LDFLAGS = -L. -L../lib -L$(LIBEVENT_LIB) -lrt -levent

AR		= ar
RANLIB	= ranlib
ARCFLAGS	= crsv
ARXFLAGS	= xv

MAIN    = core/EGC_server.o 
OBJS    = core/EGC_comm.o core/EGC_event.o core/EGC_lib.o core/EGC_utils.o \
          aspdebug/ASP_debug.o \
          communication/EGC_comm_file.o communication/EGC_comm_sock.o

HEADS   = egc_sys.h utilmacros.h

#Define Inference Rules

.c.o:
	$(CC) $(CFLAGS) -c $*.c -o $@

#Define Target

all: $(TARGET)

depos:
	mkdir depos

egcpops: libegc.a $(MAIN) 
		$(LD) $(LDFLAGS) -o $@ $(MAIN) -legc

libegc.a: depos libraries $(OBJS) 
	cd depos && $(AR) $(ARXFLAGS) ../config_reader/libcfr.a 
	cd depos && $(AR) $(ARXFLAGS) ../procmng/libpmg.a 
	$(AR) $(ARCFLAGS) $@ $(OBJS) depos/*.o
	
#List Indirect Dependence
libraries: 
	cd config_reader && make -e 
	cd procmng && make -e
	
samples:
	cd ../sample && make -e

#Meta Convert

#Useful Command
clean:
	rm -f $(TARGET) $(OBJS) $(MAIN) $(TEST) libegc.a *~
	rm -rf depos

allclean:
	rm -f $(TARGET) $(OBJS) $(MAIN) $(TEST) libegc.a *~
	rm -rf depos
	cd config_reader && make clean
	cd procmng && make clean
	cd ../sample && make clean

	

#install:
#	cp $(TARGET) $(INSLIB)
#	cp $(HEADS) $(INSINC)
