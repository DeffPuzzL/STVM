INCDIR= -I/usr/include -I$(HOME)/include -I./ -I./include -I../include
LIBDIR= -L$(HOME)/lib -L../lib -lstvm  -lm -lc -ldl -lpthread
CC=cc -fPIC
CO=-c -g
OUTLIB=../lib
OUTBIN=../bin

OBJFILE=tree.o sem.o msg.o tcp.o str.o list.o conf.o
CREATE=create
INSERT=insert
SELECT=select
QUERY=query
DELETE=delete
UPDATE=update
COUNT=count
GROUP=group
EXTREME=extreme
TRUNCATE=truncate
DROP=drop
CLICK=click
PRESSURE=press_demo

all: $(CREATE) $(INSERT) $(SELECT) $(QUERY) $(DELETE) $(UPDATE) $(COUNT) $(GROUP) $(EXTREME) $(TRUNCATE) $(DROP) $(PRESSURE) $(CLICK) clean
$(CREATE): create.o
	$(CC) -o $@ $< $(LIBDIR)
$(INSERT): insert.o
	$(CC) -o $@ $< $(LIBDIR)
$(SELECT): select.o
	$(CC) -o $@ $< $(LIBDIR)
$(QUERY): query.o
	$(CC) -o $@ $< $(LIBDIR)
$(DELETE): delete.o
	$(CC) -o $@ $< $(LIBDIR)
$(UPDATE): update.o
	$(CC) -o $@ $< $(LIBDIR)
$(COUNT): count.o
	$(CC) -o $@ $< $(LIBDIR)
$(GROUP): group.o
	$(CC) -o $@ $< $(LIBDIR)
$(EXTREME): extreme.o
	$(CC) -o $@ $< $(LIBDIR)
$(TRUNCATE): truncate.o
	$(CC) -o $@ $< $(LIBDIR)
$(DROP): drop.o
	$(CC) -o $@ $< $(LIBDIR)
$(CLICK): click.o
	$(CC) -o $@ $< $(LIBDIR)
$(PRESSURE): press_demo.o
	$(CC) -o $@ $< $(LIBDIR)

.SUFFIXES: .c .o

.c.o:
	$(CC) $(CO) $*.c $(INCDIR) $(LIBDIR)

fall:
	@touch *c
	@make all
clean:
	rm -f *.o
