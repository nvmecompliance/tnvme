# Copyright (c 2011, Intel Corporation.
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

SRC =					\
	grpQueues.cpp			\
	initialStateAdmin_r10b.cpp	\
	adminQRollChkSame_r10b.cpp	\
	adminQRollChkDiff_r10b.cpp	\
	createResources_r10b.cpp	\
	ioqRollChkSame_r10b.cpp		\
	ioqRollChkDiff_r10b.cpp		\
	manySQtoCQAssoc_r10b.cpp	\
	adminQFull_r10b.cpp		\
	ioqFull_r10b.cpp		\
	manyCmdSubmit_r10b.cpp		\
	illegalDeleteQs_r10b.cpp	\
	illegalCreateOrder_r10b.cpp	\
	maxIOQ_r10b.cpp			\
	sqcqSizeMismatch_r10b.cpp	\
	qIdVariations_r10b.cpp		\
	illegalCreateQs_r10b.cpp

.SUFFIXES: .cpp

OBJ = $(SRC:.cpp=.o)
OUT = libGrpQueues.a

all: $(OUT)

.cpp.o:
	$(CC) $(INCLUDES) $(CFLAGS) $(DFLAGS) -c $< -o $@ $(LDFLAGS)

$(OUT): $(OBJ)
	ar rcs $(OUT) $(OBJ)

clean:
	rm -f $(OBJ) Makefile.bak

clobber: clean
	rm -f $(OUT)
