# Copyright (c) 2011, Intel Corporation.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
LDFLAGS=-lm
LIBS = -L../ -L/usr/local/lib -lm
INCLUDES = -I. -I../ -I../../ -I/usr/local/include

SRC =						\
	grpBasicInit.cpp			\
	createACQASQ_r10b.cpp			\
	createIOQContigPoll_r10b.cpp		\
	createIOQContigIrq_r10b.cpp		\
	createIOQDiscontigPoll_r10b.cpp		\
	createIOQDiscontigIrq_r10b.cpp		\
	deleteIOQContig_r10b.cpp		\
	deleteIOQDiscontig_r10b.cpp		\
	writeDataPat_r10b.cpp			\
	verifyDataPat_r10b.cpp

.SUFFIXES: .cpp

OBJ = $(SRC:.cpp=.o)
OUT = libGrpBasicInit.a

all: $(OUT)

.cpp.o:
	$(CC) $(INCLUDES) $(CFLAGS) $(DFLAGS) -c $< -o $@ $(LDFLAGS)

$(OUT): $(OBJ)
	ar rcs $(OUT) $(OBJ)

clean:
	rm -f $(OBJ) Makefile.bak

clobber: clean
	rm -f $(OUT)
