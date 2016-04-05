TARGET  = libcfr.a

ifndef PFIXENV
export TMP_INC    = include
export TMP_LIB    = lib
export CFLAG_EXT  = -I ../../include/
export LDFLAG_EXT =
export PFIXENV    = 
export CC      = gcc
export LD      = gcc
endif

CFLAGS  = -c -Wall -g -I$(TMP_INC) -I. $(CFLAG_EXT) 

AR		= ar
RANLIB	= ranlib
ARCFLAGS	= crsv
ARXFLAGS	= xv

LDFLAGS = -g -o
LDFLAGS2 = -L$(TMP_LIB) -L.

OBJS	= CFR_base.o
          
HEADS   = libcfr.h

#Define Inference Rules

.c.o:
	$(CC) $(CFLAGS) $*.c
	
all: $(TMP_INC) $(TMP_LIB) $(TARGET)
#	cp $(HEADS) $(TMP_INC)
#	cp $(TARGET) $(TMP_LIB)

$(TMP_INC):
#	mkdir $(TMP_INC)
	
$(TMP_LIB):
#	mkdir $(TMP_LIB)

$(TARGET): $(OBJS)
	$(AR) $(ARCFLAGS) $@ $(OBJS)
	
	
clean:
	rm -f $(TARGET) $(OBJS) *.o *~
#	for hd in $(HEADS); do rm -f $(TMP_INC)/$$hd; done
#	for hd in $(TARGET); do rm -f $(TMP_LIB)/$$hd; done

#allclean:
#	rm -f $(TARGET) $(OBJS) *.o *~
#	cd $(ALLINC); rm -f $(HEADS)
#	cd xpt; make allclean;
