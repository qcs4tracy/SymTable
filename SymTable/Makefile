#this is a makefile
CC=$(shell which gcc)
RM=rm -f
CFLAGS=-c -Wall -std=gnu99
LDFLAGS=
GEN_FILES=tinycpp_lex.c \
	 tinycpp_parser.c \
	 tinycpp_parser.h \
	 tinycpp.output

OBJS=hashtable.o \
	tinycpp_lex.o \
	tinycpp_parser.o \
	compile_globals.o \
	compile.o \
	misc.o \
	main.o

DEPS=yacc
DEPS+=$(OBJS)
TARGET=symtable_analyze

all: $(DEPS)
	$(CC) $(LDFLAGS) $(OBJS) -o $(TARGET)

yacc:
	bison -v -d -o tinycpp_parser.c tinycpp_parser.y
	flex tinycpp_lex.l
	
%.o: %.c:
	$(CC) $(CFLAGS) $^ -o $@

clean:
	$(RM) $(OBJS) $(GEN_FILES)

.PONEY:clean
