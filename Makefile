Q=10
OS := $(shell uname)
ifeq ($(OS), Darwin)
DARWIN=true
else
DARWIN=false
endif

TAR=/usr/bin/tar
TEACHER=true
DEPS=false
CP=/bin/cp
CC=gcc
CFLAGS=-g -Wall -DTEACHER=$(TEACHER) -DDARWIN=$(DARWIN) -DQ=$(Q)
LDFLAGS=-g -DTEACHER=$(TEACHER) -pthread

PRESOURCES_1=\
cond_protected_buffer.c\
cond_protected_buffer.h\
main_executor.c\
executor.c\
executor.h\
protected_buffer.h\
protected_buffer.c\
sem_protected_buffer.h\
sem_protected_buffer.c\
thread_pool.c\
thread_pool.h\
scenario.c\
scenario.h\
utils.c\
utils.h\

SOURCES_1 = \
circular_buffer.h\
circular_buffer.c\

OBJECTS_1 = \
circular_buffer.o\
cond_protected_buffer.o\
executor.o\
main_executor.o\
protected_buffer.o\
scenario.o\
sem_protected_buffer.o\
thread_pool.o\
utils.o\

PRESOURCES = \
$(PRESOURCES_1)\

SOURCES = \
$(SOURCES_1)\

OBJECTS = \
$(OBJECTS_1)\

PROGS = \
main_executor\

%.c: %.p.c
	awk -f presources.awk -v TEACHER=$(TEACHER) $< >$@

%.h: %.p.h
	awk -f presources.awk -v TEACHER=$(TEACHER) $< >$@

.c.o:
	$(CC) -c $(CFLAGS) $<

default : $(PROGS)

clean : 
	$(RM) $(OBJECTS) $(PROGS) *~

veryclean: clean
	$(RM) $(PRESOURCES)

main_executor : $(PRESOURCES_1) $(OBJECTS_1)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS_1) 

student:
	@make veryclean
	@make TEACHER=false $(PRESOURCES)

teacher:
	@make veryclean
	@make TEACHER=true $(PRESOURCES)

index.html: index.texi
	makeinfo \
	        --no-headers --html --ifinfo --no-split \
		--css-include=style.css $< > $@

error :
	$(error "PREFIX variable not set")

install : index.html
	@if test -z "$(PREFIX)"; then \
	   make error; \
	fi
	@make student Q=10
	-mkdir -p $(PREFIX)
	chmod og=u-w $(PREFIX)
	$(TAR) zcf src.tar.gz `cat MANIFEST`
	chmod og=u-w style.css index.html src.tar.gz
	cp -p style.css index.html src.tar.gz $(PREFIX)

deps: $(SOURCES) $(PRESOURCES)
	$(CC) -M $(SOURCES) $(PRESOURCES) >deps

-include deps
