CC=g++
CFLAGS=-g -O0 -W -Wall -Werror -DDEBUG
LDFLAGS=-lm
LIBS = -L../ -L/usr/local/lib -lm
INCLUDES = -I. -I../ -I../../ -I/usr/local/include

SRC = \
	grpInformative.cpp			\
	dumpPciAddrSpace_r10a.cpp	\
	dumpCtrlrAddrSpace_r10a.cpp

.SUFFIXES: .cpp

OBJ = $(SRC:.cpp=.o)
OUT = libGrpInformative.a

all: $(OUT)

.cpp.o:
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

$(OUT): $(OBJ)
	ar rcs $(OUT) $(OBJ)

clean:
	rm -f $(OBJ) Makefile.bak

clobber: clean
	rm -f $(OUT)
