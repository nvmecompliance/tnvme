CC=g++
CFLAGS=-g -O0 -W -Wall -Werror -DDEBUG
APP_NAME:=tnvme
LDFLAGS=$(foreach stem, $(SUBDIRS),./$(stem)/lib$(stem).a)
INCLUDE := -I./ -I../

SUBDIRS:=				\
	Singletons 			\
	GrpInformative		\
	GrpPciRegisters		\
	GrpCtrlRegisters

SOURCES:=				\
	globals.cpp			\
	group.cpp			\
	test.cpp			\
	testDescribe.cpp	\
	tnvme.cpp			\
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
	rm -rf rpm
	rm dump.reg.*
	rm -f $(APP_NAME)

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
	mkdir -p $(SRCDIR)/dnvme
	(git archive HEAD) | tar xf - -C $(SRCDIR)
	git archive --remote=ssh://dcgshare.lm.intel.com/share/lm/repo/nvme/dnvme HEAD dnvme_interface.h | tar xf - -C $(SRCDIR)/dnvme
	git archive --remote=ssh://dcgshare.lm.intel.com/share/lm/repo/nvme/dnvme HEAD dnvme_ioctls.h | tar xf - -C $(SRCDIR)/dnvme

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

.PHONY: all clean clobber doc $(SUBDIRS) src install rpmzipsrc rpmbuild
