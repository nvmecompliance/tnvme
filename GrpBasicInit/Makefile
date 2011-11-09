CC=g++
CFLAGS=-g -O0 -W -Wall -Werror -DDEBUG
LDFLAGS=-lm
LIBS = -L../ -L/usr/local/lib -lm
INCLUDES = -I. -I../ -I../../ -I/usr/local/include

SRC = \
	grpBasicInit.cpp		\
	createACQASQ_r10b.cpp	\
	identifyCmd_r10b.cpp

.SUFFIXES: .cpp

OBJ = $(SRC:.cpp=.o)
OUT = libGrpBasicInit.a

all: $(OUT)

.cpp.o:
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

$(OUT): $(OBJ)
	ar rcs $(OUT) $(OBJ)

clean:
	rm -f $(OBJ) Makefile.bak

clobber: clean
	rm -f $(OUT)
