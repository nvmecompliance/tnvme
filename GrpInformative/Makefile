# source files.
SRC = \
	grpInformative.cpp			\
	dumpPciAddrSpace_r10a.cpp	\
	dumpCtrlrAddrSpace_r10a.cpp

.SUFFIXES: .cpp

OBJ = $(SRC:.cpp=.o)
OUT = ../libGrpInformative.a

CCFLAGS = -g -W -Wall -Werror
LDFLAGS=-lm
CC = g++

LIBS = -L../ -L/usr/local/lib -lm
INCLUDES = -I. -I../ -I/usr/local/include

all: $(OUT)

.cpp.o:
	$(CC) $(INCLUDES) $(CCFLAGS) -c $< -o $@ $(LDFLAGS)

$(OUT): $(OBJ)
	ar rcs $(OUT) $(OBJ)

clean:
	rm -f $(OBJ) Makefile.bak

clobber: clean
	rm -f $(OUT)
