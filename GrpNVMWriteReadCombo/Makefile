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
	grpNVMWriteReadCombo.cpp		\
	createResources_r10b.cpp		\
	prpOffsetSinglePgSingleBlk_r10b.cpp	\
	prpOffsetSinglePgMultiBlk_r10b.cpp	\
	prpOffsetDualPgMultiBlk_r10b.cpp	\
	prpOffsetMultiPgMultiBlk_r10b.cpp	\
	startingLBABare_r10b.cpp		\
	nlbaBare_r10b.cpp			\
	datasetMgmt_r10b.cpp			\
	startingLBAMeta_r10b.cpp		\
	nlbaMeta_r10b.cpp			\
	prp2Rsvd_r10b.cpp

.SUFFIXES: .cpp

OBJ = $(SRC:.cpp=.o)
OUT = libGrpNVMWriteReadCombo.a

all: $(OUT)

.cpp.o:
	$(CC) $(INCLUDES) $(CFLAGS) $(DFLAGS) -c $< -o $@ $(LDFLAGS)

$(OUT): $(OBJ)
	ar rcs $(OUT) $(OBJ)

clean:
	rm -f $(OBJ) Makefile.bak

clobber: clean
	rm -f $(OUT)
