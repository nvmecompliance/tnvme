SHELL:=/bin/bash 
CC=g++
CFLAGS=-g -W -Wall -Werror
APP_NAME:=tnvme

SUBDIRS:= \
	GrpInformative		\
	GrpCtrlRegisters

SOURCES:= \
	group.cpp \
	test.cpp \
	testDescribe.cpp \
	tnvme.cpp \
	tnvmeHelpers.cpp

#
# RPM build parameters
#
RPMBASE=tnvme
MAJOR=$(shell awk 'FNR==5' version.h)
MINOR=$(shell awk 'FNR==8' version.h)
SOFTREV=$(MAJOR).$(MINOR)
RPMFILE=$(RPMBASE)-$(SOFTREV)
RPMCOMPILEDIR=$(PWD)/rpmbuild
RPMSRCFILE=$(PWD)/$(RPMFILE)
RPMSPECFILE=$(RPMBASE).spec
SRCDIR?=./src

LDFLAGS=-lm $(patsubst Grp%, libGrp%.a, $(SUBDIRS))

INCLUDE := ./

all: GOAL=all
all: $(APP_NAME) doc

rpm: rpmzipsrc rpmbuild

clean: GOAL=clean
clean: $(SUBDIRS)
	rm -f *.o
	rm -f doxygen.log
	rm -rf $(SRCDIR)
	rm -rf $(RPMFILE)
	rm -rf $(RPMCOMPILEDIR)
	rm -rf $(RPMSRCFILE)
	rm -f $(RPMSRCFILE).tar*

clobber: GOAL=clobber
clobber: $(SUBDIRS) clean
	rm -rf doc
	rm -f $(APP_NAME)
	rm -f *.a

doc: GOAL=doc
doc: $(SUBDIRS)
	doxygen doxygen.conf > doxygen.log

$(SUBDIRS):
	$(MAKE) -C $@ $(GOAL)

$(APP_NAME): $(SUBDIRS) $(SOURCES)
	$(CC) $(CFLAGS) -I$(INCLUDE) $(SOURCES) -o $(APP_NAME) $(LDFLAGS)

# Specify a custom source c:ompile dir: "make src SRCDIR=../compile/dir"
# If the specified dir could cause recursive copies, then specify w/o './'
# "make src SRCDIR=src" will copy all except "src" dir.
src:
	rm -rf $(SRCDIR)
	mkdir -p $(SRCDIR)
	(git archive HEAD) | tar xf - -C $(SRCDIR)

install:
	# typically one invokes this as "sudo make install"
	install -p tnvme $(DESTDIR)/usr/bin

rpmzipsrc: SRCDIR:=$(RPMFILE)
rpmzipsrc: clobber src
	rm -f $(RPMSRCFILE).tar*
	tar cvf $(RPMSRCFILE).tar $(RPMFILE)
	gzip $(RPMSRCFILE).tar

rpmbuild: rpmzipsrc
	# Build the RPM and then copy the results local
	./build.sh $(RPMCOMPILEDIR) $(RPMSPECFILE) $(RPMSRCFILE)
	rm -rf ./rpm
	mkdir ./rpm
	cp -p $(RPMCOMPILEDIR)/RPMS/x86_64/*.rpm ./rpm
	cp -p $(RPMCOMPILEDIR)/SRPMS/*.rpm ./rpm

.PHONY: all clean doc $(SUBDIRS) src rpmzipsrc
