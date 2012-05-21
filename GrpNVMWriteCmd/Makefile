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
	grpNVMWriteCmd.cpp			\
	createResources_r10b.cpp		\
	lbaOutOfRangeBare_r10b.cpp		\
	invalidNamspc_r10b.cpp			\
	unsupportRsvdFields_r10b.cpp		\
	ignoreMetaPtrBare_r10b.cpp		\
	protInfoIgnoreBare_r10b.cpp		\
	FUA_r10b.cpp				\
	limitedRetry_r10b.cpp			\
	lbaOutOfRangeMeta_r10b.cpp		\
	ignoreMetaPtrMeta_r10b.cpp		\
	protInfoIgnoreMeta_r10b.cpp

.SUFFIXES: .cpp

OBJ = $(SRC:.cpp=.o)
OUT = libGrpNVMWriteCmd.a

all: $(OUT)

.cpp.o:
	$(CC) $(INCLUDES) $(CFLAGS) $(DFLAGS) -c $< -o $@ $(LDFLAGS)

$(OUT): $(OBJ)
	ar rcs $(OUT) $(OBJ)

clean:
	rm -f $(OBJ) Makefile.bak

clobber: clean
	rm -f $(OUT)
