INCDIR= -I/usr/include -I$(HOME)/include -I./ -I./include -I../include
#LIBDIR= -L$(HOME)/lib -L../lib -lm -lc -ldl -lpthread -static -lstvm
LIBDIR= -L$(HOME)/lib -L../lib -lstvm -lm -lc -ldl -lrt -lpthread 
CC=cc -fPIC -g
CO=-c -pg
OUTLIB=../lib
OUTBIN=../bin

OBJFILE=tree.o sem.o msg.o tcp.o str.o list.o conf.o
CREATE=create_table
QUEUE=create_queue
PUSH=push
POP=pop
TVMPOP=tvmpop
TVMREPLACE=tvmreplace
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
REPLACE=replace
POPUP=popup
PRESSURE=press_demo
TVMPUSH=tvmpush
TVMINSERT=tvminsert
ASYPUSH=asypush
THREAD=thread_demo

all: $(CREATE) $(INSERT) $(QUEUE) $(PUSH) $(POP) $(SELECT) $(QUERY) $(DELETE) $(UPDATE) $(COUNT) $(GROUP) $(EXTREME) $(TRUNCATE) $(DROP) $(PRESSURE) $(CLICK) $(POPUP) $(REPLACE) $(TVMPUSH) $(ASYPUSH) $(TVMPOP) $(TVMREPLACE) $(THREAD) $(TVMINSERT) clean
$(CREATE): create_table.o
	$(CC) -o $@ $< $(LIBDIR)
$(TVMINSERT): tvminsert.o
	$(CC) -o $@ $< $(LIBDIR)
$(TVMREPLACE): tvmreplace.o
	$(CC) -o $@ $< $(LIBDIR)
$(TVMPOP): tvmpop.o
	$(CC) -o $@ $< $(LIBDIR)
$(PUSH): push.o
	$(CC) -o $@ $< $(LIBDIR)
$(TVMPUSH): tvmpush.o
	$(CC) -o $@ $< $(LIBDIR)
$(ASYPUSH): asypush.o
	$(CC) -o $@ $< $(LIBDIR)
$(POP): pop.o
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
$(REPLACE): replace.o
	$(CC) -o $@ $< $(LIBDIR)
$(QUEUE): create_queue.o
	$(CC) -o $@ $< $(LIBDIR)
$(POPUP): popup.o
	$(CC) -o $@ $< $(LIBDIR)
$(PRESSURE): press_demo.o
	$(CC) -o $@ $< $(LIBDIR)
$(THREAD): thread_demo.o
	$(CC) -o $@ $< $(LIBDIR)

.SUFFIXES: .c .o

.c.o:
	$(CC) $(CO) $*.c $(INCDIR) $(LIBDIR)

fall:
	@touch *c
	@make all
clean:
	rm -f *.o
