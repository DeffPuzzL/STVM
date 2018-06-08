INCDIR= -I/usr/include -I$(HOME)/include -I./ -I./include -I../include
LIBDIR= -L$(HOME)/lib -L../lib -lstvm -lreadline -lm -lc -ldl -lpthread
AR=ar
LD=ld
LN=ln
#CC=cc -fPIC -O2 -Wno-unused-result -m64 -march=native
#CC=cc -fPIC -O2 -Wno-unused-result -march=native
CC=cc -fPIC -Wno-unused-result 
CO=-c -g -DMAX_INDEX_LEN=64 -DMAX_FILED_NUM=64 -Wno-unused-result 
LDFLAG=-shared -g
OUTLIB=../lib
OUTBIN=../bin

OBJFILE=tree.o sem.o msg.o tcp.o str.o list.o conf.o queue.o
TARGET=$(OUTLIB)/libstvm.a
#TARDLL=$(OUTLIB)/libstvm.so
TARVER=$(OUTLIB)/libstvm.so.1.2
STVM=$(OUTBIN)/stvm
DETVM=$(OUTBIN)/detvm

all: $(TARGET) $(TARDLL) $(STVM) $(DETVM) clean
$(TARGET): $(OBJFILE)
	rm -f $@
	mkdir -p $(OUTLIB)
	$(AR) -cr $@ $(OBJFILE)

$(TARDLL): $(OBJFILE)
	$(LD) $(LDFLAG) -o $@ $(OBJFILE) 
	mkdir -p $(OUTLIB)
	rm -f $(TARVER)
	$(LN) -s $@ $(TARVER)

$(STVM): stvm.o
	$(CC) -o $@ $< $(LIBDIR)

$(DETVM): detvm.o
	$(CC) -o $@ $< $(LIBDIR)

.SUFFIXES: .c .o

.c.o:
	$(CC) $(CO) $*.c $(INCDIR) $(LIBDIR)

fall:
	@touch *c
	@make all
clean:
	rm -f *.o
