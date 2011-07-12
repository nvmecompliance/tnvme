CC=g++
CFLAGS=-g -W -Wall -Werror
APP_NAME := tnvme

SUBDIRS := \
	GrpCtrlRegisters

SOURCES := \
	group.cpp \
	test.cpp \
	testDescribe.cpp \
	tnvme.cpp

LDFLAGS=-lm $(patsubst Grp%, libGrp%.a, $(SUBDIRS))

INCLUDE := ./

all: GOAL=all
all: $(APP_NAME) doc

clean: GOAL=clean
clean: $(SUBDIRS)
	rm -f *.o
	rm -f $(APP_NAME)
	rm -f doxygen.log
	rm -f *.a

doc: GOAL=doc
doc: $(SUBDIRS)
	doxygen doxygen.conf > doxygen.log

$(SUBDIRS):
	$(MAKE) -C $@ $(GOAL)

$(APP_NAME): $(SUBDIRS) $(SOURCES)
	$(CC) $(CFLAGS) -I$(INCLUDE) $(SOURCES) -o $(APP_NAME) $(LDFLAGS)

.PHONY: all clean doc $(SUBDIRS)
